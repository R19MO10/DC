/**
* @file LoadIcon.h
*/
#ifndef LOADICON_H_INCLUDED
#define LOADICON_H_INCLUDED

// ��s�錾
struct GLFWimage;

/**
* .ico�t�@�C����ǂݍ���ŁAGLFWimage�\���̂��쐬����֐�
*
* @param filename �ǂݍ���.ico�t�@�C���̃p�X
*
* @return �쐬���ꂽGLFWimage�\����
*/
GLFWimage LoadIcon_as_GLFWimage(const char* filename);

#endif // LOADICON_H_INCLUDED