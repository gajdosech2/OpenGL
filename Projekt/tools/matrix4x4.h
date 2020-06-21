
class Matrix4x4 {


public:

	float matrix[16];

	Matrix4x4();

	void Translate(float dx, float dy, float dz);

	void setPerspective( float angle_of_view,
    float aspect_ratio,
    float z_near,
    float z_far);

	void Scale(float x, float y, float z);

	void setOrtho(float left, float right, float top, float bottom, float near, float far);

	void Rotate(float angle, float x, float y, float z); 

	void LoadIdentity();

};