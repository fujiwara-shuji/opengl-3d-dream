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

- **座標系**: 右手系、Z 軸上、X 軸右、Y 軸奥
- **面の表裏**: 左回り（CCW）= 表面
- **行列**: OpenGL 標準（列優先）
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

- **反射式**: R = I - 2 _ (I・N) _ N
- **反射率**: 表面・裏面で個別設定可能
- **深度制限**: 最大反射回数で無限ループ防止
- **offsetPoint**: 数値誤差対策で交点を法線方向に微小移動

### 環境光

- **光源方向**: (1, -1, 1)正規化
- **色計算**: 光線方向と光源方向の内積でグラデーション
- **色設定**: 暗部色・明部色を線形補間

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
- **反射設定**: 反射有効/無効、表面・裏面反射率調整

### メニューバー

- **File**: 新規作成、開く、保存、名前を付けて保存、終了
- **Edit**: 元に戻す、やり直し、頂点追加・削除
- **View**: プリセット視点、選択頂点フォーカス
- **Help**: 操作方法、このソフトについて

## 描画設定

### 色設定

- **頂点色**: 通常（白）、選択中（オレンジ）
- **辺色**: 薄いグレー
- **面色**: 表面（濃いグレー）、裏面（より濃いグレー）
- **背景**: 環境光グラデーション

### 表示制御

- **頂点**: 表示/非表示切り替え
- **辺**: 表示/非表示切り替え
- **面**: 表示/非表示、表面/裏面個別制御
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

### メインレイキャスティング処理

```cpp
Vector3 castRay(const Ray& ray, const Model& model, int depth = 0) {
    const int MAX_REFLECTION_DEPTH = 5;

    if (depth >= MAX_REFLECTION_DEPTH) {
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

            if (useReflection) {
                // 反射ベクトル計算
                Vector3 reflectedDir = reflect(ray.direction, hit.normal);

                // 反射光線を少し面から離して生成（数値誤差対策）
                Vector3 offsetPoint = hit.point + hit.normal * 0.001f;
                Ray reflectedRay(offsetPoint, reflectedDir);

                // 再帰的に反射色を計算
                Vector3 reflectedColor = castRay(reflectedRay, model, depth + 1);

                float alpha = hit.isFrontFace ? frontFaceReflectionAlpha : backFaceReflectionAlpha;
                Vector3 baseColor = hit.isFrontFace ? frontFaceColor : backFaceColor;

                return baseColor * (1.0f - alpha) + reflectedColor * alpha;
            } else {
                return hit.isFrontFace ? frontFaceColor : backFaceColor;
            }
        }

        case RaycastResult::NONE:
        default:
            return calculateSkyboxColor(ray);
    }
}
```

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
