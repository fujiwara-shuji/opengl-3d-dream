# 3D モデル編集ソフトウェア設計仕様書

## プロジェクト概要

レイトレーシングを用いた Blender 風の 3D モデル編集ソフトウェアの開発。学習目的でソフトウェアレンダリングによる独自実装を行う。

## 技術要件

### 開発環境

- **言語**: C++
- **環境**: WSL（Windows/Linux 対応）
- **ライブラリ**:
  - GLFW（ウィンドウ作成・入力処理）
  - Dear ImGui（UI）
  - OpenGL（最終画面表示のみ）
- **メモリ管理**: std::vector 使用
- **エラー処理**: シンプルな実装（複雑な例外処理なし）

### 座標系・規約

- **座標系**: 左手系、Z 軸上、X 軸右、Y 軸手前
- **面の表裏**: 左回り（CCW）= 表面
- **行列**: OpenGL 標準（列優先、左手座標系変換済み）
- **インデックス**: 0 始まり

## ファイル形式

### .fjwr 形式（独自テキスト形式）

```
# コメント行
v 0.0 0.0 0.0    # 頂点（x y z）
v 1.0 0.0 0.0
v 1.0 1.0 0.0
f 0 1 2          # 面（頂点インデックス3個）
l 0 1            # 辺（頂点インデックス2個）
```

## アーキテクチャ設計

### クラス構成（更新版）

```
Application（メインアプリケーション）
├── Model（データ管理・ファイルI/O）
├── Camera（視点操作）
├── CoordinateAxes（座標軸表示・管理）
├── IRenderer（レンダリングインターフェース）
│   └── SoftwareRenderer（ソフトウェアレイトレーシング実装）
├── InputHandler（マウス・キーボード処理）
├── UI（Dear ImGuiツールパネル）
├── RenderSettings（描画設定管理）
└── Utils（エラーハンドリング・ユーティリティ）
```

### データフロー

```
ファイル(.fjwr) ⇔ Model ⇔ Renderer ⇒ 画面表示
                    ↕
                 Camera（視点）
                    ↕
                InputHandler（操作）
                    ↕
                   UI（設定）
```

## 基本構造体

### 数学ライブラリ

```cpp
struct Vector3 {
    float x, y, z;
    // 基本演算、正規化、内積、外積等
};

class Matrix4 {
    float m[16];  // 列優先
    // 行列演算、変換行列生成等
};
```

### 3D データ

```cpp
struct Vertex {
    Vector3 position;
    Vector3 normal;
};

struct Face {
    int v1, v2, v3;  // 頂点インデックス
};

struct Edge {
    int v1, v2;      // 頂点インデックス
};

struct Ray {
    Vector3 origin;
    Vector3 direction;
};
```

## レンダリングシステム

### ソフトウェアレイトレーシング

- **方式**: CPU で 1 ピクセルずつ色計算
- **フレームレート**: 30fps 固定
- **解像度**: 可変（ウィンドウサイズに応じて）

### 交差判定ロジック

1. **頂点**: 光線と点の距離が閾値以下
2. **辺**: 光線と線分の最短距離が閾値以下
3. **面**: 光線と三角形の交差（表裏判定あり）

優先順位: 距離が最も近いもの

### 反射計算

#### ランバート拡散反射（Lambert Diffuse Reflection）

物体表面の基本的な明暗を表現する拡散反射モデル。

- **基本原理**: ランバートの余弦則
  - 表面が受ける光の強度は、法線と光源方向の角度のコサインに比例
  - `NdotL = max(0, dot(normal, -lightDirection))`

- **色計算**:
  ```cpp
  Vector3 ambient = baseColor * ambientStrength;     // 環境光成分
  Vector3 diffuse = baseColor * NdotL * diffuseStrength;  // 拡散反射成分
  finalColor = ambient + diffuse;
  ```

- **パラメータ**:
  - `enableLambertDiffuse`: ランバート反射の有効/無効
  - `lightDirection`: 光源方向ベクトル（デフォルト: (1, -1, 1)正規化）
  - `ambientStrength`: 環境光の強さ（0.0 - 1.0、デフォルト: 0.2）
  - `diffuseStrength`: 拡散反射の強さ（0.0 - 1.0、デフォルト: 0.8）

- **特徴**:
  - 自然な立体感と陰影を表現
  - 光源方向を向いた面は明るく、背を向けた面は暗くなる
  - 視点の位置に依存しない（常に同じ明るさ）

#### 鏡面反射（Specular Reflection）

鏡のような光沢のある表面を表現する反射モデル。

- **反射式**: R = I - 2 * (I・N) * N
- **反射率**: 表面・裏面で個別設定可能
  - `frontFaceReflectionAlpha`: 表面の反射強度（0.0 - 1.0）
  - `backFaceReflectionAlpha`: 裏面の反射強度（0.0 - 1.0）
- **深度制限**: 最大反射回数で無限ループ防止（デフォルト: 5回）
- **offsetPoint**: 数値誤差対策で交点を法線方向に微小移動

#### 反射モデルの組み合わせ

ランバート反射と鏡面反射を組み合わせることで、リアルな表面を表現：

1. **ランバート反射のみ**:
   - 自然な陰影のみ
   - マットな表面の表現

2. **鏡面反射のみ**:
   - 鏡のような完全反射
   - 金属やガラスの表現

3. **両方有効**（推奨）:
   ```cpp
   // Step 1: ランバート反射で基本的な明暗を計算
   Vector3 lambertColor = ambient + diffuse;

   // Step 2: 鏡面反射色を再帰的に計算
   Vector3 specularColor = castRay(reflectedRay, depth + 1);

   // Step 3: 両者をブレンド
   finalColor = lambertColor * (1 - reflectionAlpha) + specularColor * reflectionAlpha;
   ```
   - 光沢のある物体（プラスチック、塗装面など）
   - よりリアルな見た目

### 環境光（スカイボックス）

反射光線が物体にヒットしなかった場合の背景色。

- **光源方向**: (1, -1, 1)正規化
- **色計算**: 光線方向と光源方向の内積でグラデーション
- **色設定**: 暗部色・明部色を線形補間
  - 暗部色: Vector3(0.1, 0.1, 0.2) - 暗い青
  - 明部色: Vector3(0.8, 0.9, 1.0) - 明るい青

## カメラシステム

### パラメータ

```cpp
class Camera {
    Vector3 target;     // 注視点
    float distance;     // 注視点からの距離
    float pitch, yaw;   // 上下・左右角度
    Vector3 up;         // 上方向（固定：0,0,1）
    float fov, aspectRatio, nearPlane, farPlane;
};
```

### 操作方法

- **ミドルクリック + ドラッグ**: 視点回転
- **ホイール**: ズーム（距離調整）
- **数字キー**: プリセット視点（1=正面、3=右側面、7=上面）

## 入力システム

### マウス操作

- **左クリック**: 頂点選択（可視性チェック付き）
- **選択閾値**: カメラ距離に応じて動的調整
- **選択解除**: 選択中頂点から一定距離以上離れた場所をクリック

### 頂点編集

- **G キー**: 選択頂点の移動モード開始
- **マウス移動**: カメラ視線に垂直な平面上で頂点移動
- **左クリック**: 移動確定
- **Escape**: 移動キャンセル

### ショートカットキー

- **Ctrl+N**: 新規作成
- **Ctrl+O**: 開く
- **Ctrl+S**: 上書き保存
- **Ctrl+Shift+S**: 名前を付けて保存
- **F**: 選択頂点にフォーカス

## UI システム

### ウィンドウレイアウト

```
┌─────────────────────────────────────┐
│ メニューバー                        │
├─────────────────┬───────────────────┤
│                 │ ツールパネル      │
│   3D表示エリア   │ ┌─────────────────┐│
│                 │ │ 表示項目切り替え ││
│                 │ └─────────────────┘│
│                 │ ┌─────────────────┐│
│                 │ │ モデル情報      ││
│                 │ └─────────────────┘│
│                 │ ┌─────────────────┐│
│                 │ │ 選択中の頂点    ││
│                 │ └─────────────────┘│
│                 │ ┌─────────────────┐│
│                 │ │ 表示設定        ││
│                 │ └─────────────────┘│
│                 │ ┌─────────────────┐│
│                 │ │ 反射設定        ││
│                 │ └─────────────────┘│
└─────────────────┴───────────────────┘
```

### ツールパネル機能

- **表示項目切り替え**: チェックボックスでセクション表示制御
- **統一スクロール**: 全体で 1 つのスクロールバー
- **モデル情報**: 頂点数、面数、辺数、ファイル名、変更状態
- **選択情報**: 選択頂点のインデックス・座標、操作ヒント
- **表示設定**: 頂点・辺・面・表裏面の表示 ON/OFF
- **反射設定**:
  - **Lambert Diffuse Reflection**（ランバート拡散反射）
    - Enable Lambert Diffuse: ON/OFF切り替え
    - Ambient Strength: 環境光の強さ調整
    - Diffuse Strength: 拡散反射の強さ調整
    - Light Direction: 光源方向の調整（X, Y, Z）
  - **Specular Reflection**（鏡面反射）
    - Enable Specular Reflection: ON/OFF切り替え
    - Max Reflection Depth: 反射の最大回数
  - **Surface Colors**（表面色）
    - Front/Back Face Reflection Alpha: 表面・裏面の反射率
    - Front/Back Face Color: 表面・裏面の基本色
  - **Advanced Settings**（高度な設定）
    - Reflection Epsilon: 自己交差回避のための微小オフセット

### メニューバー

- **File**: 新規作成、開く、保存、名前を付けて保存、終了
- **Edit**: 元に戻す、やり直し、頂点追加・削除
- **View**: プリセット視点、選択頂点フォーカス
- **Help**: 操作方法、このソフトについて

## 座標軸システム

### 概要

3D空間の向きを視覚的に理解するための座標軸表示システム。左手座標系でX軸（赤）、Y軸（緑）、Z軸（青）を原点から各軸方向に表示する。

### 実装仕様

#### 座標軸の定義（左手座標系）
- **X軸**: 赤色 (1.0, 0.0, 0.0) - 右方向
- **Y軸**: 緑色 (0.0, 1.0, 0.0) - 手前方向
- **Z軸**: 青色 (0.0, 0.0, 1.0) - 上方向
- **基点**: 原点 (0, 0, 0)
- **長さ**: 設定可能（デフォルト 2.0単位）

#### 3D描画による隠蔽処理
座標軸は3Dオブジェクトとして扱われ、正しい隠蔽処理を行う：

```cpp
// Line構造体（座標軸の線分表現）
struct Line {
    Vector3 start;      // 線分開始点
    Vector3 end;        // 線分終了点
    Vector3 color;      // 線の色
    float thickness;    // 線の太さ
};

// Ray-Line交差判定（辺と同様のアルゴリズム）
LineHit intersectLine(const Ray& ray, const Line& line, float threshold, int lineIndex) {
    // rayEdgeDistance()を活用
    // 線の太さを考慮した閾値計算
    // 正確な交差点とパラメータを返す
}
```

#### レンダリング統合
SoftwareRenderer で線分と面の距離比較により正確な描画順序を実現：

```cpp
Vector3 castRay(const Ray& ray) const {
    float closestDistance = std::numeric_limits<float>::max();
    Vector3 hitColor;

    // 1. ライン（座標軸）との交差判定
    for (const Line& line : lines) {
        LineHit hit = RayIntersection::intersectLine(ray, line, threshold, index);
        if (hit.hit && hit.distance < closestDistance) {
            closestDistance = hit.distance;
            hitColor = line.color;
        }
    }

    // 2. 三角形（面）との交差判定
    for (const Triangle& triangle : triangles) {
        TriangleHit hit = RayIntersection::intersectTriangle(ray, triangle.v0, v1, v2);
        if (hit.hit && hit.distance < closestDistance) {
            closestDistance = hit.distance;
            hitColor = triangle.color * shading;
        }
    }

    return hitColor;  // 最前面のオブジェクトの色を返す
}
```

### CoordinateAxes クラス設計

```cpp
class CoordinateAxes {
private:
    std::vector<Line> axisLines;    // 3本の軸線
    bool showAxes;                  // 表示/非表示
    float axisLength;               // 軸の長さ
    float axisThickness;            // 軸の太さ
    Vector3 xAxisColor, yAxisColor, zAxisColor;  // 各軸の色

public:
    // 設定管理
    void setVisible(bool visible);
    void setAxisLength(float length);
    void setAxisThickness(float thickness);
    void setAxisColors(const Vector3& x, const Vector3& y, const Vector3& z);

    // 軸データアクセス
    const std::vector<Line>& getAxisLines() const;
    void regenerateAxes();  // 設定変更時の軸再生成

private:
    void createAxisLines();  // 3本の軸線を生成
};
```

### 操作仕様

#### キーボードコントロール
- **Aキー**: 座標軸表示の切り替え（表示/非表示）
- **+キー**: 軸の長さを 0.5単位 増加
- **-キー**: 軸の長さを 0.5単位 減少（最小 0.5単位）

#### UI統合（Phase 6 予定）
```cpp
// Dear ImGui での座標軸設定パネル
void UI::renderAxisSettings() {
    if (ImGui::CollapsingHeader("Coordinate Axes")) {
        ImGui::Checkbox("Show Axes", &showAxes);

        if (showAxes) {
            ImGui::SliderFloat("Axis Length", &axisLength, 0.5f, 5.0f);
            ImGui::SliderFloat("Axis Thickness", &axisThickness, 0.5f, 3.0f);
            ImGui::ColorEdit3("X Axis Color", &xAxisColor.x);
            ImGui::ColorEdit3("Y Axis Color", &yAxisColor.x);
            ImGui::ColorEdit3("Z Axis Color", &zAxisColor.x);
        }
    }
}
```

### 技術的特徴

#### 正確な3D描画
- レイトレーシングによる正確な距離計算
- 面による軸の遮蔽を正しく処理
- 軸が他のオブジェクトの背後にある場合は表示されない

#### 効率的な実装
- 既存の `rayEdgeDistance` アルゴリズムを再利用
- Model クラスと独立した設計
- SoftwareRenderer への軽量な統合

#### 拡張性
- 軸の色・長さ・太さのカスタマイズ
- 将来的な軸ラベル表示への対応
- グリッド表示への拡張可能性

#### 左手座標系の確認方法
左手の親指・人差し指・中指を直角に伸ばすとき：
- 親指: X軸（右方向）
- 人差し指: Y軸（手前方向）
- 中指: Z軸（上方向）

視点操作での確認：
- 1キー（正面視点）: X軸（赤）が右、Z軸（青）が上
- 3キー（右側面視点）: Y軸（緑）が手前、Z軸（青）が上
- 7キー（上面視点）: X軸（赤）が右、Y軸（緑）が手前

## 描画設定

### 色設定

- **頂点色**: 通常（白）、選択中（オレンジ）
- **辺色**: 薄いグレー
- **面色**: 表面（濃いグレー）、裏面（より濃いグレー）
- **座標軸色**: X軸（赤）、Y軸（緑）、Z軸（青）
- **背景**: 環境光グラデーション

### 表示制御

- **頂点**: 表示/非表示切り替え
- **辺**: 表示/非表示切り替え
- **面**: 表示/非表示、表面/裏面個別制御
- **座標軸**: 表示/非表示切り替え、長さ・太さ調整
- **反射**: 有効/無効、反射率調整

## 実装の優先順位

### Phase 0: プロジェクト構造（追加）

1. **プロジェクト構造作成**

   ```
   opengl-3d-dream/
   ├── CMakeLists.txt
   ├── CLAUDE.md
   ├── src/
   │   ├── main.cpp
   │   ├── Application.h/cpp
   │   ├── math/
   │   │   ├── Vector3.h/cpp
   │   │   └── Matrix4.h/cpp
   │   ├── core/
   │   │   ├── Model.h/cpp
   │   │   ├── Camera.h/cpp
   │   │   └── Ray.h
   │   ├── rendering/
   │   │   └── IRenderer.h
   │   │   └── SoftwareRenderer.h/cpp
   │   ├── input/
   │   │   └── InputHandler.h/cpp
   │   ├── ui/
   │   │   └── UI.h/cpp
   │   └── utils/
   │       └── Utils.h/cpp
   ├── include/
   ├── external/
   │   ├── glfw/
   │   └── imgui/
   └── build/
   ```

2. **CMakeLists.txt 設定**

   - GLFW、Dear ImGui の依存関係設定
   - C++17 設定
   - ビルド設定

3. **基本的な main.cpp 骨格**
   - Application クラスのインスタンス化
   - 初期化・メインループ・終了処理の呼び出し

### Phase 1: 基盤システム

1. Vector3, Matrix4 クラス実装
2. ウィンドウ作成・OpenGL コンテキスト
3. 基本的な描画（単色画面表示）

### Phase 2: レイトレーシング基本

1. Ray クラス、交差判定関数
2. 面との交差判定実装
3. 単純な面描画

### Phase 3: カメラ・入力

1. Camera クラス実装
2. マウス・キーボード入力
3. 視点操作

### Phase 4: モデルデータ・ファイル I/O

1. Model クラス実装
2. .fjwr ファイル読み書き
3. 基本図形生成

### Phase 5: 完全な交差判定

1. 頂点・辺の交差判定
2. 可視性チェック
3. 選択機能

### Phase 5.5: 座標軸システム（実装済み）

1. **Line構造体と交差判定**
   - Line 構造体の定義
   - LineHit 結果構造体
   - RayIntersection::intersectLine() 実装

2. **CoordinateAxes クラス**
   - 3軸（X:赤、Y:緑、Z:青）の生成・管理
   - 表示設定と動的な長さ・太さ調整
   - 軸線データのレンダラー統合

3. **SoftwareRenderer 拡張**
   - ライン描画機能の追加
   - 線分と三角形の統合された距離判定
   - 正確な隠蔽処理の実装

4. **Phase5Test 統合**
   - 座標軸の表示確認
   - キーボードコントロール（A, +, -キー）
   - リアルタイム設定変更

### Phase 6: UI・編集機能

1. Dear ImGui ツールパネル
2. 頂点移動機能
3. メニューバー

### Phase 7: 反射・最適化

1. 反射計算実装
2. パフォーマンス調整
3. バグ修正・ポリッシュ

## 重要なアルゴリズム実装例

### 光線と辺の距離計算

```cpp
float calculateRayEdgeDistance(const Ray& ray, const Edge& edge) {
    Vector3 P0 = ray.origin;
    Vector3 d = ray.direction;
    Vector3 A = edge.start;
    Vector3 B = edge.end;
    Vector3 AB = B - A;

    // Step 1: 線分を無限直線として扱い、最近点を求める
    Vector3 P0A = A - P0;
    float t = Vector3::dot(P0A, AB) / Vector3::dot(AB, AB);

    // Step 2: 線分の範囲内にクランプ
    t = std::clamp(t, 0.0f, 1.0f);

    // Step 3: 線分上の最近点
    Vector3 closestOnEdge = A + t * AB;

    // Step 4: 光線上で、この点に最も近い点を求める
    Vector3 P0ToClosest = closestOnEdge - P0;
    float rayParam = Vector3::dot(P0ToClosest, d);

    // 重要：光線は後ろには伸びない
    rayParam = std::max(rayParam, 0.0f);

    Vector3 closestOnRay = P0 + rayParam * d;

    // Step 5: 距離を計算
    return (closestOnRay - closestOnEdge).length();
}
```

### 三角形内部判定（左手側判定）

```cpp
bool isPointInsideTriangle(const Vector3& point,
                          const Vector3& v0,
                          const Vector3& v1,
                          const Vector3& v2,
                          const Vector3& normal) {
    // 各辺に対して点が左手側にあるかチェック

    // 辺AB (v0 → v1)
    Vector3 edge1 = v1 - v0;
    Vector3 toPoint1 = point - v0;
    Vector3 cross1 = Vector3::cross(edge1, toPoint1);
    if (Vector3::dot(cross1, normal) < 0) return false;

    // 辺BC (v1 → v2)
    Vector3 edge2 = v2 - v1;
    Vector3 toPoint2 = point - v1;
    Vector3 cross2 = Vector3::cross(edge2, toPoint2);
    if (Vector3::dot(cross2, normal) < 0) return false;

    // 辺CA (v2 → v0)
    Vector3 edge3 = v0 - v2;
    Vector3 toPoint3 = point - v2;
    Vector3 cross3 = Vector3::cross(edge3, toPoint3);
    if (Vector3::dot(cross3, normal) < 0) return false;

    return true; // 全ての辺で左手側なら内部
}
```

### 三角形との交差判定（完全版）

```cpp
struct TriangleHit {
    bool hit;
    float distance;
    Vector3 point;
    Vector3 normal;
    bool isFrontFace;
};

TriangleHit intersectTriangle(const Ray& ray,
                             const Vector3& v0,
                             const Vector3& v1,
                             const Vector3& v2) {
    TriangleHit result;
    result.hit = false;

    // 法線計算（左回り = CCW）
    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;
    Vector3 normal = Vector3::cross(edge1, edge2).normalized();

    // 表裏判定
    float denom = Vector3::dot(normal, ray.direction);
    bool isFrontFace = (denom < 0);  // 負なら表面

    if (abs(denom) < 1e-6f) return result; // 平行

    // 平面との交点
    Vector3 toPlane = v0 - ray.origin;
    float t = Vector3::dot(toPlane, normal) / denom;
    if (t < 0) return result; // 光線の後ろ側

    Vector3 point = ray.origin + t * ray.direction;

    // 三角形内部判定
    if (!isPointInsideTriangle(point, v0, v1, v2, normal)) {
        return result;
    }

    result.hit = true;
    result.distance = t;
    result.point = point;
    result.normal = isFrontFace ? normal : -normal;  // 裏面なら法線を反転
    result.isFrontFace = isFrontFace;
    return result;
}
```

### 反射ベクトル計算

```cpp
Vector3 reflect(const Vector3& incident, const Vector3& normal) {
    // 反射の公式: R = I - 2 * (I・N) * N
    // I: 入射ベクトル, N: 法線ベクトル, R: 反射ベクトル

    float dotProduct = Vector3::dot(incident, normal);
    return incident - 2.0f * dotProduct * normal;
}
```

### ランバート拡散反射計算

```cpp
Vector3 calculateLambertDiffuse(
    const Vector3& hitPoint,
    const Vector3& normal,
    const Vector3& baseColor,
    const ReflectionConfig& config
) {
    // ランバートの余弦則: 光の強度は法線と光源方向の角度のコサインに比例
    float NdotL = std::max(0.0f, Vector3::dot(normal, -config.lightDirection));

    // 環境光成分: 常に一定の明るさを保証
    Vector3 ambient = baseColor * config.ambientStrength;

    // 拡散反射成分: 光源方向に依存
    Vector3 diffuse = baseColor * NdotL * config.diffuseStrength;

    // 最終色 = 環境光 + 拡散反射
    return ambient + diffuse;
}
```

**パラメータ説明**:
- `normal`: 表面の法線ベクトル（正規化済み）
- `lightDirection`: 光源から表面への方向ベクトル（正規化済み）
- `NdotL`: 法線と光源方向の内積（コサイン値）
  - 1.0: 光源を正面から受ける → 最も明るい
  - 0.0: 光源と垂直 → 拡散反射なし（環境光のみ）
  - 負の値: 光源が裏側 → 0にクランプ
- `ambientStrength`: 環境光の強さ（通常 0.1 - 0.3）
- `diffuseStrength`: 拡散反射の強さ（通常 0.7 - 0.9）

### カメラ座標変換（スクリーン → ワールド光線）

```cpp
Ray Camera::screenToWorldRay(double screenX, double screenY, int windowWidth, int windowHeight) const {
    // スクリーン座標を正規化座標に変換（-1〜1）
    float x = (2.0f * screenX / windowWidth) - 1.0f;
    float y = 1.0f - (2.0f * screenY / windowHeight);

    // 正規化デバイス座標からワールド座標へ
    Matrix4 invViewProj = (getProjectionMatrix() * getViewMatrix()).inverse();

    Vector3 nearPoint(x, y, -1.0f);  // near plane上の点
    Vector3 farPoint(x, y, 1.0f);    // far plane上の点

    // ワールド座標に変換
    Vector3 worldNear = invViewProj * nearPoint;
    Vector3 worldFar = invViewProj * farPoint;

    Vector3 direction = (worldFar - worldNear).normalized();

    return Ray(getPosition(), direction);
}
```

### 動的閾値計算（カメラ距離対応）

```cpp
class InputHandler {
private:
    const float BASE_SELECTION_THRESHOLD = 0.1f;  // 基準距離での閾値
    const float BASE_DESELECTION_THRESHOLD = 0.2f;

public:
    float getScaledSelectionThreshold(const Camera& camera) const {
        float cameraDistance = camera.getDistance();
        return BASE_SELECTION_THRESHOLD * cameraDistance * 0.1f;
    }

    float getScaledDeselectionThreshold(const Camera& camera) const {
        float cameraDistance = camera.getDistance();
        return BASE_DESELECTION_THRESHOLD * cameraDistance * 0.1f;
    }
};
```

### 可視性チェック（頂点選択用）

```cpp
bool isVertexVisible(const Vector3& cameraPos, const Vector3& vertex, const Model& model) {
    // カメラから頂点への光線を作成
    Vector3 direction = (vertex - cameraPos).normalized();
    float targetDistance = (vertex - cameraPos).length();
    Ray visibilityRay(cameraPos, direction);

    // 面との交差をチェック
    const auto& faces = model.getFaces();
    const auto& vertices = model.getVertices();

    for (const auto& face : faces) {
        Vector3 v0 = vertices[face.v1];
        Vector3 v1 = vertices[face.v2];
        Vector3 v2 = vertices[face.v3];

        TriangleHit hit = intersectTriangle(visibilityRay, v0, v1, v2);

        // 頂点より手前で面と交差している場合は隠蔽されている
        if (hit.hit && hit.distance < (targetDistance - 0.001f)) {
            return false;
        }
    }

    return true; // 隠蔽されていない
}
```

### 環境光色計算

```cpp
Vector3 calculateSkyboxColor(const Ray& ray) {
    Vector3 skyLightDirection = Vector3(1, -1, 1).normalized();  // 光源方向
    Vector3 skyDarkColor = Vector3(0.1f, 0.1f, 0.2f);          // 暗い部分の色
    Vector3 skyBrightColor = Vector3(0.8f, 0.9f, 1.0f);        // 明るい部分の色

    // 光線方向と光源方向の角度で色を決定
    float alignment = Vector3::dot(ray.direction, skyLightDirection);

    // -1〜1を0〜1にマッピング
    float t = (alignment + 1.0f) * 0.5f;

    // 線形補間で色を計算
    return Vector3::lerp(skyDarkColor, skyBrightColor, t);
}
```

### メインレイキャスティング処理（ランバート反射 + 鏡面反射）

```cpp
Vector3 castRay(const Ray& ray, const Model& model, int depth = 0) {
    // 最大反射深度チェック
    if (depth >= reflectionConfig.maxReflectionDepth) {
        return calculateSkyboxColor(ray);
    }

    // 最も近い交差点を見つける
    RaycastResult result = findClosestIntersection(ray, model, depth > 0);

    switch (result.type) {
        case RaycastResult::VERTEX:
            return getVertexColor(result.elementIndex);

        case RaycastResult::EDGE:
            return getEdgeColor();

        case RaycastResult::FACE: {
            TriangleHit hit = result.triangleHit;

            // 基本色を決定
            Vector3 baseColor = hit.isFrontFace ?
                reflectionConfig.frontFaceColor :
                reflectionConfig.backFaceColor;

            Vector3 finalColor = baseColor;

            // Step 1: ランバート拡散反射を適用
            if (reflectionConfig.enableLambertDiffuse) {
                // ランバートの余弦則
                float NdotL = std::max(0.0f,
                    Vector3::dot(hit.normal, -reflectionConfig.lightDirection));

                // 環境光 + 拡散反射
                Vector3 ambient = baseColor * reflectionConfig.ambientStrength;
                Vector3 diffuse = baseColor * NdotL * reflectionConfig.diffuseStrength;

                finalColor = ambient + diffuse;
            }

            // Step 2: 鏡面反射を適用（ランバート反射の上にブレンド）
            if (reflectionConfig.enableReflection) {
                // 反射ベクトル計算
                Vector3 reflectedDir = Vector3::reflect(ray.direction, hit.normal);

                // 反射光線を少し面から離して生成（数値誤差対策）
                Vector3 offsetPoint = hit.point + hit.normal * reflectionConfig.reflectionEpsilon;
                Ray reflectedRay(offsetPoint, reflectedDir);

                // 再帰的に反射色を計算
                Vector3 reflectedColor = castRay(reflectedRay, model, depth + 1);

                // 表面・裏面で反射率を変更
                float reflectionAlpha = hit.isFrontFace ?
                    reflectionConfig.frontFaceReflectionAlpha :
                    reflectionConfig.backFaceReflectionAlpha;

                // ランバート色と鏡面反射色をブレンド
                finalColor = finalColor * (1.0f - reflectionAlpha) +
                            reflectedColor * reflectionAlpha;
            }

            return finalColor;
        }

        case RaycastResult::NONE:
        default:
            return calculateSkyboxColor(ray);
    }
}
```

**処理フロー**:
1. **深度チェック**: 無限ループ防止
2. **交差判定**: 最も近いオブジェクトを特定
3. **ランバート反射**: 基本的な明暗を計算
   - 環境光: 最低限の明るさを保証
   - 拡散反射: 光源方向に応じた明暗
4. **鏡面反射**: 光沢を追加
   - 反射光線を再帰的に追跡
   - ランバート色とブレンド
5. **最終色**: ランバート反射 + 鏡面反射の合成

## 技術的注意点

### パフォーマンス

- 小さなモデルから開始（数十頂点程度）
- 必要に応じて空間分割等の最適化検討

### 数値精度

- 浮動小数点誤差対策（epsilon 値使用）
- offsetPoint 実装で自己交差回避

### 可視性判定

- 頂点選択時の隠蔽チェック実装
- レイキャスティングによる遮蔽判定

### メモリ管理

- std::vector 活用
- RAII パターンで OpenGL リソース管理

## 将来の拡張可能性

- 複数オブジェクト対応
- アンドゥ・リドゥ機能
- より高度なシェーディング
- 空間分割による高速化

## 練習用ソフト向け実装指針（追加）

### 最小限の必須要素

#### 1. 基本的なエラーハンドリング

```cpp
// utils/Utils.h
#pragma once
#include <iostream>
#include <string>

namespace Utils {
    // 基本的なアサーション
    #define ASSERT(condition, message) \
        if (!(condition)) { \
            std::cerr << "ERROR: " << message << std::endl; \
            exit(1); \
        }

    // OpenGLエラーチェック
    void checkGLError(const char* operation);

    // ファイル存在チェック
    bool fileExists(const std::string& path);

    // 簡単なエラーログ
    void logError(const std::string& message);
    void logInfo(const std::string& message);
}
```

#### 2. レンダリングインターフェース

```cpp
// rendering/IRenderer.h
#pragma once
#include "../core/Model.h"
#include "../core/Camera.h"

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void render(const Model& model, const Camera& camera) = 0;
    virtual void setResolution(int width, int height) = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

// rendering/SoftwareRenderer.h
#pragma once
#include "IRenderer.h"
#include "RenderSettings.h"

class SoftwareRenderer : public IRenderer {
private:
    RenderSettings settings;
    // 既存のレンダリング実装

public:
    void render(const Model& model, const Camera& camera) override;
    void setResolution(int width, int height) override;
    void initialize() override;
    void shutdown() override;

    // 設定アクセス
    RenderSettings& getSettings() { return settings; }
};
```

#### 3. 設定管理

```cpp
// rendering/RenderSettings.h
#pragma once

struct RenderSettings {
    // 表示制御
    bool showVertices = true;
    bool showEdges = true;
    bool showFaces = true;
    bool showFrontFaces = true;
    bool showBackFaces = true;

    // 反射設定
    bool enableReflection = false;
    float frontFaceReflectionAlpha = 0.3f;
    float backFaceReflectionAlpha = 0.1f;
    int maxReflectionDepth = 5;

    // 色設定
    struct Colors {
        Vector3 vertexNormal = Vector3(1.0f, 1.0f, 1.0f);      // 白
        Vector3 vertexSelected = Vector3(1.0f, 0.5f, 0.0f);    // オレンジ
        Vector3 edge = Vector3(0.7f, 0.7f, 0.7f);              // 薄いグレー
        Vector3 frontFace = Vector3(0.5f, 0.5f, 0.5f);         // 濃いグレー
        Vector3 backFace = Vector3(0.3f, 0.3f, 0.3f);          // より濃いグレー
    } colors;

    // 閾値設定
    float baseSelectionThreshold = 0.1f;
    float baseDeselectionThreshold = 0.2f;
};
```

#### 4. Application クラスの更新

```cpp
// Application.h
#pragma once
#include <memory>
#include <GLFW/glfw3.h>
#include "core/Model.h"
#include "core/Camera.h"
#include "rendering/IRenderer.h"
#include "input/InputHandler.h"
#include "ui/UI.h"

class Application {
private:
    GLFWwindow* window = nullptr;
    std::unique_ptr<Model> model;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<IRenderer> renderer;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<UI> ui;

    // ウィンドウサイズ
    int windowWidth = 1280;
    int windowHeight = 720;

    // 基本的な状態管理
    bool shouldClose = false;

public:
    bool initialize();
    void run();
    void shutdown();

private:
    void setupCallbacks();
    void update();
    void render();

    // コールバック関数
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void errorCallback(int error, const char* description);
};
```

#### 5. 基本的なファイル I/O エラーハンドリング

```cpp
// core/Model.h に追加
class Model {
public:
    bool loadFromFile(const std::string& path) {
        if (!Utils::fileExists(path)) {
            Utils::logError("File not found: " + path);
            return false;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            Utils::logError("Cannot open file: " + path);
            return false;
        }

        // ファイル形式チェック
        if (path.substr(path.find_last_of(".") + 1) != "fjwr") {
            Utils::logError("Unsupported file format. Expected .fjwr");
            return false;
        }

        // 既存の読み込み処理...
        return true;
    }

    bool saveToFile(const std::string& path) {
        std::ofstream file(path);
        if (!file.is_open()) {
            Utils::logError("Cannot create file: " + path);
            return false;
        }

        // 既存の保存処理...
        return true;
    }
};
```

### 実装時の注意事項

1. **段階的な実装**: Phase 0 から順番に実装し、各段階で動作確認を行う
2. **最小限の機能**: 完璧を求めず、動作する最小限の機能から開始
3. **デバッグ出力**: 開発中は適度にログ出力を入れて動作確認
4. **エラー処理**: ファイル I/O、OpenGL 操作、メモリ確保で基本的なエラーチェック
5. **設定の外部化**: ハードコード値を避け、RenderSettings で管理
