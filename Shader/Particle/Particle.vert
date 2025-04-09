/**
* @file Particle.vert
*/
#version 450

// シェーダへの入力
layout(location=0) in vec3 inPosition;  // 頂点座標
layout(location=1) in vec2 inTexcoord;  // テクスチャ座標

// シェーダからの出力
layout(location=0) out vec4 outColor;   // 基本色
layout(location=1) out vec2 outTexcoord;// テクスチャ座標

// プログラムからの入力
layout(location=1) uniform mat4 cameraTransformMatrix;  // カメラの座標変換行列

// x: 視野角による水平拡大率
// y: 視野角による垂直拡大率
// z: 遠近法パラメータA
// w: 遠近法パラメータB
layout(location=2) uniform vec4 cameraInfo;

// パーティクル
struct ParticleData
{
	vec3 position;	// 座標
	float rotation;	// 回転
	
	vec4 color;		// 色と不透明度

	vec4 direction;	// xyz: 方向, w: 方向使用の有無(0, 1)

	vec2 size;		// 拡大縮小
	float damy[2];  // 64バイト区切りにするための埋め合わせ
};
// パーティクル用SSBO
layout(std430, binding=2) buffer particleDataBlock
{
    ParticleData particleList[];
};


/**
* エントリーポイント
*/
void main()
{
    // パーティクルの色を設定
    ParticleData particle = particleList[gl_InstanceID];
    outColor = particle.color;

    // プリミティブのテクスチャ座標を計算
    outTexcoord = inTexcoord;

    // パーティクルの回転行列を作成
    mat3 rot = mat3(1);
    const float c = cos(particle.rotation);
    const float s = sin(particle.rotation);
    rot[0][0] = c;
    rot[0][1] = s;
    rot[1][0] = -s;
    rot[1][1] = c;
    rot[0] *= particle.size.x;
    rot[1] *= particle.size.y;

    // カメラの座標変換行列から回転行列を取得
    const mat3 cameraRotation = mat3(cameraTransformMatrix);

    if (particle.direction.w == 0){
        // 方向を使用しない場合（ビルボード）
        rot = cameraRotation * rot; 
    }
    else {
        // 方向を使用する場合（方向ベクトルや中心からのベクトル）

        // オイラー角を抽出
        const float yaw =   // Y軸
		    atan(cameraRotation[2][0], cameraRotation[2][2]);

	    const float sinY = sin(yaw);
	    const float cosY = cos(yaw);
	    const float roll = // Z軸
		    atan(sinY * cameraRotation[1][2] - cosY * cameraRotation[1][0],
			    cosY * cameraRotation[0][0] - sinY * cameraRotation[0][2]);

        const float sinZ = sin(roll);
        const float cosZ = cos(roll);
	    const mat3 mz = {
	    	{ cosZ,		sinZ,		0	  },
	    	{ -sinZ,	cosZ,		0	  },
	    	{ 0,		0,			1	  }
        };
        
        const vec3 direction = normalize(vec3(particle.direction));
                       
        // y軸との外積を取り、x軸を求める
        const vec3 xAxis = normalize(cross(direction, vec3(0.0, 1.0, 0.0))); 
        // 方向ベクトルとx軸の外積を取り、y軸を求める
        const vec3 yAxis = normalize(cross(xAxis, direction)); 
        // z軸は方向ベクトルと同じ
        const vec3 zAxis = direction;
        
        mat3 rotateToDirection = mat3(xAxis, yAxis, zAxis);
        
        // X軸に90度回転させる用の回転行列
        const mat3 rotateY90 = mat3(
            vec3(0.0, 0.0, -1.0),
            vec3(0.0, 1.0,  0.0),
            vec3(1.0, 0.0,  0.0)
        );
        rotateToDirection = rotateToDirection * mz * rotateY90;
       
        rot = rotateToDirection * rot;
    }
  
    const mat4 transMat = mat4(
        vec4(rot[0], 0.0), 
        vec4(rot[1], 0.0), 
        vec4(rot[2], 0.0), 
        vec4(particle.position, 1)
    );

    gl_Position = transMat * vec4(inPosition, 1);

    // ワールド座標系からビュー座標系に変換
    vec3 pos = gl_Position.xyz - vec3(cameraTransformMatrix[3]);
    gl_Position.xyz = pos * mat3(cameraTransformMatrix);
  
    // 視野角を反映
    gl_Position.xy *= cameraInfo.xy;
  
    // 遠近法を有効にする
    gl_Position.w = -gl_Position.z;
  
    // 深度値を補正（深度値の計算結果が-1～+1になるようにする）
    gl_Position.z = -gl_Position.z * cameraInfo.w + cameraInfo.z;
}