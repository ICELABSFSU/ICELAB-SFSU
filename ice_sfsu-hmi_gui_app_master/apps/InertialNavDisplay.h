#pragma once
ref class InertialNavDisplay
{
private:
	static const int A = 60;
	static const int R = 15;
	static const int dotsize = 4;
	static System::Drawing::SolidBrush^ brush;
	static System::Drawing::SolidBrush^ brushref;
	static System::Drawing::Pen^ penvbar;
	static System::Drawing::Pen^ penheading;
	static System::Drawing::Pen^ penroll;
	static System::Drawing::Pen^ penpitchref;
	static System::Drawing::Pen^ penref;
	static System::Drawing::Pen^ penvec;

	inline static void vbarpos(int points[5][2], float pitch, float roll, int xcenter, int ycenter);
public:
	InertialNavDisplay();
	virtual ~InertialNavDisplay();

	// pitch and roll in radians
	static System::Void drawAttitude(System::Windows::Forms::PictureBox^ pictureBox, float pitch, float roll);
	// yaw in radians
	static System::Void drawHeading(System::Windows::Forms::PictureBox^ pictureBox, float yaw);
	// pitch, roll, and yaw in radians
	static System::Void drawCombined(System::Windows::Forms::PictureBox^ pictureBox, float pitch, float roll, float yaw);
	// acceleration in g
	static System::Void drawAccelerationXY(System::Windows::Forms::PictureBox^ pictureBox, float accelx, float accely);
	// acceleration in g
	static System::Void drawAccelerationZVert(System::Windows::Forms::PictureBox^ pictureBox, float accelz);
};

