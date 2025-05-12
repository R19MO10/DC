# DC

#include <iostream>
#include <Eigen/Dense>
#include <map>
#include <cmath>
#include <optional>

using namespace Eigen;
using namespace std;

Vector3f faceToVector(int face) {
    switch (face) {
        case 1: return Vector3f(0, 0, 1);   // front
        case 2: return Vector3f(1, 0, 0);   // right
        case 3: return Vector3f(0, 1, 0);   // top
        case 4: return Vector3f(-1, 0, 0);  // left
        case 5: return Vector3f(0, -1, 0);  // bottom
        case 6: return Vector3f(0, 0, -1);  // back
        default: return Vector3f(0, 0, 0);
    }
}

bool isOrthogonal(const Vector3f& a, const Vector3f& b) {
    return abs(a.dot(b)) < 1e-5; // 直交チェック
}

optional<Vector3f> computeEulerXYZ(int frontFace, int topFace) {
    Vector3f forward = faceToVector(frontFace);
    Vector3f up = faceToVector(topFace);

    if (forward.norm() == 0 || up.norm() == 0 || !isOrthogonal(forward, up)) {
        return nullopt;
    }

    Vector3f right = up.cross(forward);
    Matrix3f rot;
    rot.col(0) = right.normalized();
    rot.col(1) = up.normalized();
    rot.col(2) = forward.normalized();

    Vector3f euler = rot.eulerAngles(0, 1, 2); // XYZ順（ラジアン）
    return euler * (180.0f / M_PI); // 度に変換
}

int main() {
    int front, top;
    cout << "正面の数字 (1〜6): ";
    cin >> front;
    cout << "上面の数字 (1〜6): ";
    cin >> top;

    auto result = computeEulerXYZ(front, top);
    if (result) {
        Vector3f angles = *result;
        cout << "回転角度 (XYZ順, 度):\n";
        cout << "X: " << angles.x() << "°, Y: " << angles.y() << "°, Z: " << angles.z() << "°\n";
    } else {
        cout << "無効な面の組み合わせです（直交していない）\n";
    }

    return 0;
}
















#include <iostream>
#include <map>
#include <tuple>
#include <optional>

using Rotation = std::tuple<int, int, int>; // x, y, z in degrees

// 初期状態：前1、上3、右2、左4、下5、後6
// 回転結果から front と top のペアで回転角を取得
std::optional<Rotation> getEulerRotation(int desiredFront, int desiredTop) {
    static const std::map<std::pair<int, int>, Rotation> orientationToEuler = {
        {{1, 3}, {0, 0, 0}},
        {{1, 2}, {90, 0, 0}},
        {{1, 5}, {180, 0, 0}},
        {{1, 4}, {270, 0, 0}},

        {{2, 3}, {0, 270, 0}},
        {{2, 1}, {90, 270, 0}},
        {{2, 5}, {180, 270, 0}},
        {{2, 6}, {270, 270, 0}},

        {{3, 6}, {0, 0, 270}},
        {{3, 2}, {0, 90, 270}},
        {{3, 1}, {0, 180, 270}},
        {{3, 4}, {0, 270, 270}},

        {{4, 3}, {0, 90, 0}},
        {{4, 6}, {90, 90, 0}},
        {{4, 5}, {180, 90, 0}},
        {{4, 1}, {270, 90, 0}},

        {{5, 1}, {0, 180, 0}},
        {{5, 2}, {90, 180, 0}},
        {{5, 6}, {180, 180, 0}},
        {{5, 4}, {270, 180, 0}},

        {{6, 3}, {0, 0, 90}},
        {{6, 4}, {0, 90, 90}},
        {{6, 5}, {0, 180, 90}},
        {{6, 2}, {0, 270, 90}},
    };

    auto key = std::make_pair(desiredFront, desiredTop);
    auto it = orientationToEuler.find(key);
    if (it != orientationToEuler.end()) {
        return it->second;
    }
    return std::nullopt;
}

// 使用例
int main() {
    int front, top;
    std::cout << "正面の数字を入力してください (1〜6): ";
    std::cin >> front;
    std::cout << "上面の数字を入力してください (1〜6): ";
    std::cin >> top;

    auto rotation = getEulerRotation(front, top);
    if (rotation) {
        auto [x, y, z] = *rotation;
        std::cout << "回転角度 (XYZ順): X=" << x << "°, Y=" << y << "°, Z=" << z << "°\n";
    } else {
        std::cout << "その組み合わせは無効です（直交する面を指定してください）。\n";
    }

    return 0;
}