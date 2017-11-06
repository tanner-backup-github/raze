#ifndef GL_MATH_H__
#define GL_MATH_H__

#define IDENTITY_MATRIX { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }

void ortho(float m[16], float left, float right, float top, float bottom,
	   float near, float far) {	
	m[0] = 2.0 / (right - left);
	m[5] = 2.0 / (top - bottom);
	m[10] = -2.0 / (far - near);
	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(far + near) / (far - near);
}

void model_matrix(float m[16], float x, float y, float w, float h, float r) {
	m[0] = cos(r) * w;
	m[1] = sin(r) * h;
	m[4] = -sin(r) * w;
	m[5] = cos(r) * h;
	m[12] = x + w / 2;
	m[13] = y + h / 2;
}

#endif
