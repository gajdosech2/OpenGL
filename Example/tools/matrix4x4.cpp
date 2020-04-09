#include "matrix4x4.h"


#define PI 3.14159265


float degToRad(float a){
	return (a * PI / 180.0);
}



float* mulM4x4(float* m1, float * m2){

	float * m;
	m = new float[16]; 

	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++){

				m[i + 4*j] = 0;
				
				for(int k=0;k<4;k++) m[i + 4*j] += m1[i + 4*k] * m2[k + 4*j];

		}

		return m;
}



Matrix4x4::Matrix4x4(){

	matrix[0] = 1.0;	matrix[1] = 0.0;	matrix[2] = 0.0;	matrix[3] = 0.0;
	matrix[4] =0.0;	matrix[5] = 1.0;	matrix[6] = 0.0;	matrix[7] = 0.0;
	matrix[8] =0.0;	matrix[9] = 0.0;	matrix[10] = 1.0;	matrix[11] = 0.0;
	matrix[12] =0.0;	matrix[13] = 0.0;	matrix[14] =0.0;	matrix[15] = 1.0;


}

void Matrix4x4::Scale(float x, float y, float z){

	float * m;
	m = new float[16]; 


	m[0] = x;	m[1] = 0.0;	m[2] = 0.0;	m[3] = 0.0;
	m[4] =0.0;	m[5] = y;	m[6] = 0.0;	m[7] = 0.0;
	m[8] =0.0;	m[9] = 0.0;	m[10] = z;	m[11] = 0.0;
	m[12] =0.0;	m[13] = 0.0;	m[14] = 0.0;	m[15] = 1.0;

	float * m1 = mulM4x4(matrix, m);

	for(int i = 0; i < 15; i++) matrix[i] = m1[i];

	delete[] m;
	delete[] m1;
}

void Matrix4x4::LoadIdentity(){

  matrix[0] = 1.0;	matrix[1] = 0.0;	matrix[2] = 0.0;	matrix[3] = 0.0;
	matrix[4] =0.0;	matrix[5] = 1.0;	matrix[6] = 0.0;	matrix[7] = 0.0;
	matrix[8] =0.0;	matrix[9] = 0.0;	matrix[10] = 1.0;	matrix[11] = 0.0;
	matrix[12] =0.0;	matrix[13] = 0.0;	matrix[14] =0.0;	matrix[15] = 1.0;
	
}

void Matrix4x4::Translate(float dx, float dy, float dz){

	float * m;
	m = new float[16]; 


	m[0] = 1.0;	m[1] = 0.0;	m[2] = 0.0;	m[3] = 0.0;
	m[4] =0.0;	m[5] = 1.0;	m[6] = 0.0;	m[7] = 0.0;
	m[8] =0.0;	m[9] = 0.0;	m[10] = 1.0;	m[11] = 0.0;
	m[12] =dx;	m[13] = dy;	m[14] =dz;	m[15] = 1.0;

	float * m1 = mulM4x4(matrix, m);

	for(int i = 0; i < 15; i++) matrix[i] = m1[i];

	delete[] m;
	delete[] m1;
	

}

void Matrix4x4::setPerspective( float angle_of_view,
    float aspect_ratio,
    float z_near,
	float z_far){

	float angle = degToRad(angle_of_view);

    matrix[0] = 1.0/(tan(angle)*aspect_ratio);	matrix[1] = 0.0;	matrix[2] = 0.0;	matrix[3] =  0.0;
    matrix[4] = 0.0;	matrix[5] = 1.0/tan(angle);	matrix[6] = 0.0;	matrix[7] = 0.0;
    matrix[8] = 0.0;	matrix[9] = 0.0;	matrix[10] = -(z_far+z_near)/(z_far-z_near);	matrix[11] = -1.0;
    matrix[12] = 0.0;	matrix[13] = 0.0;	matrix[14] = -2.0*z_far*z_near/(z_far-z_near);	matrix[15] = 0.0;

}

void Matrix4x4::setOrtho(float left, float right, float top, float bottom, float z_near, float z_far){

	//http://www.songho.ca/opengl/gl_projectionmatrix.html

	matrix[0] = 2.0/(right-left);	matrix[1] = 0.0;	matrix[2] = 0.0;	matrix[3] = 0.0;
	matrix[4] =0.0;	matrix[5] = 2.0/(top-bottom);	matrix[6] = 0.0;	matrix[7] = 0.0;
	matrix[8] =0.0;	matrix[9] = 0.0;	matrix[10] = -2.0/(z_far-z_near);	matrix[11] = 0.0;
	matrix[12] =(-right-left)/(right-left);	matrix[13] = (-top-bottom)/(top-bottom);	matrix[14] = (-z_far-z_near)/(z_far-z_near);	matrix[15] = 1.0;

}

void Matrix4x4::Rotate(float angle, float x, float y, float z){

		//http://www.cprogramming.com/tutorial/3d/rotation.html


	float c = cos(degToRad(angle));
	float s = sin(degToRad(angle));
	float t = 1.0 - c;
	
	float len = sqrt(x*x + y*y + z*z);
  x = x / len; 
  y = y / len; 
  z = z / len; 


	float * m;
	m = new float[16]; 


	m[0] = t*x*x+c;	m[1] = t*x*y-s*z;	m[2] = t*x*z+s*y;	m[3] = 0.0;
	m[4] = t*x*y+s*z;	m[5] = t*y*y+c;	m[6] = t*y*z-s*x;	m[7] = 0.0;
	m[8] = t*x*z-s*y;	m[9] = t*y*z+s*x;	m[10] = t*z*z+c;	m[11] = 0.0;
	m[12] = 0.0;	m[13] = 0.0;	m[14] =0.0;	m[15] = 1.0;

	float * m1 = mulM4x4(matrix, m);

	for(int i = 0; i < 15; i++) matrix[i] = m1[i];

	delete[] m;
	delete[] m1;


}


