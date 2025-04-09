/**
* @file LoadIcon.h
*/
#ifndef LOADICON_H_INCLUDED
#define LOADICON_H_INCLUDED

// 先行宣言
struct GLFWimage;

/**
* .icoファイルを読み込んで、GLFWimage構造体を作成する関数
*
* @param filename 読み込む.icoファイルのパス
*
* @return 作成されたGLFWimage構造体
*/
GLFWimage LoadIcon_as_GLFWimage(const char* filename);

#endif // LOADICON_H_INCLUDED