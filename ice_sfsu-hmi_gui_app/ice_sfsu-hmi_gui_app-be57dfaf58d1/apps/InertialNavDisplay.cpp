#include "InertialNavDisplay.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;
using namespace System::Windows::Forms;

inline void InertialNavDisplay::vbarpos(int vbar[5][2], float pitch, float roll, int xcenter, int ycenter)
{
	//a----b o d----e
	//      \ /
	//       c
	ycenter -= pitch;

	double cosroll = Math::Cos(roll);
	double sinroll = Math::Sin(roll);
	int ax = A * cosroll;
	int ay = A * sinroll;
	int rx = R * cosroll;
	int ry = R * sinroll;

	//a
	vbar[0][0] = xcenter - ax;
	vbar[0][1] = ycenter - ay;
	//b
	vbar[1][0] = xcenter - rx;
	vbar[1][1] = ycenter - ry;
	//c
	vbar[2][0] = xcenter - ry;
	vbar[2][1] = ycenter + rx;
	//d
	vbar[3][0] = xcenter + rx;
	vbar[3][1] = ycenter + ry;
	//e
	vbar[4][0] = xcenter + ax;
	vbar[4][1] = ycenter + ay;
}

InertialNavDisplay::InertialNavDisplay()
{
	brush = gcnew System::Drawing::SolidBrush(System::Drawing::Color::Black);
	brushref = gcnew System::Drawing::SolidBrush(System::Drawing::Color::Coral);
	penvbar = gcnew Pen(Color::Black);
	penvbar->Width = 2;
	penvbar->StartCap = LineCap::Triangle;
	penvbar->EndCap = LineCap::Triangle;
	penvbar->LineJoin = LineJoin::Miter;
	penheading = gcnew Pen(Color::Black);
	penheading->Width = 5;
	penheading->StartCap = LineCap::Triangle;
	penheading->EndCap = LineCap::Triangle;
	penroll = gcnew Pen(Color::Black);
	penroll->Width = 8;
	penroll->StartCap = LineCap::Triangle;
	penpitchref = gcnew Pen(Color::White);
	penpitchref->Width = 1;
	penref = gcnew Pen(Color::Black);
	penref->Width = 1;
	penvec = gcnew Pen(Color::Black);
	penvec->Width = 5;
	penvec->StartCap = LineCap::RoundAnchor;
	penvec->EndCap = LineCap::ArrowAnchor;
}

InertialNavDisplay::~InertialNavDisplay()
{
	delete brush;
	delete brushref;
	delete penvbar;
	delete penheading;
	delete penroll;
	delete penpitchref;
	delete penref;
	delete penvec;
}

System::Void InertialNavDisplay::drawAttitude(PictureBox^ pictureBox, float pitch, float roll)
{
	int vbar[5][2];
	array<Point>^ points = gcnew array<Point>(5);
	int w = pictureBox->Size.Width;
	int h = pictureBox->Size.Height;
	int xcenter = w / 2;
	int ycenter = h / 2;
	int pitchscaled = h * pitch / Math::PI;
	vbarpos(vbar, pitchscaled, roll, xcenter, ycenter);
	int atty = ycenter - pitchscaled;

	Bitmap^ bitmap = gcnew Bitmap(w, h);
	Graphics^ g = Graphics::FromImage(bitmap);
	g->Clear(Color::DodgerBlue);

	g->FillRectangle(brushref, 0, ycenter, w, ycenter);

	//pitch ref
	{
		//horizon
		Pen^ horiz = gcnew Pen(Color::White);
		horiz->Width = 2;
		g->DrawLine(horiz,
			0, ycenter,
			w, ycenter);

		//top and bottom half (add ycenter * i)
		for (int i = 0; i < 2; i++)
		{
			int div = 90 / 10;
			int y;
			for (int j = 1; j < div; j++)
			{
				y = ycenter * j / div + ycenter * i;
				g->DrawLine(penpitchref,
					xcenter - R, y,
					xcenter + R, y);
			}
		}

		int wq = w / 4;
		int wq2 = 3 * wq;
		int hq = h / 4;
		//+45deg
		g->DrawLine(penpitchref,
			wq, hq,
			wq2, hq);
		int hq2 = hq * 3;
		//-45deg
		g->DrawLine(penpitchref,
			wq, hq2,
			wq2, hq2);
	}

	//roll ref
	{
		int div = 360 / 30;
		float divref = Math::PI * 2 / div;
		int radius = xcenter < ycenter ? xcenter : ycenter;
		for (int i = 0; i < div; i++)
		{
			double cosroll = Math::Cos(i * divref);
			double sinroll = Math::Sin(i * divref);
			g->DrawLine(penpitchref,
				(int)(xcenter + radius * cosroll),
				(int)(ycenter + radius * sinroll),
				(int)(xcenter + (radius - 10) * cosroll),
				(int)(ycenter + (radius - 10) * sinroll));
		}

		//roll angle
		double cosroll = -Math::Cos(roll);
		double sinroll = Math::Sin(roll);
		g->DrawLine(penroll,
			(int)(xcenter + (radius - 10) * sinroll),
			(int)(ycenter + (radius - 10) * cosroll),
			(int)(xcenter + (radius - 16) * sinroll),
			(int)(ycenter + (radius - 16) * cosroll));

	}

	//center dot
	g->FillEllipse(brush,
		xcenter - dotsize / 2, atty - dotsize / 2,
		dotsize, dotsize
		);

	//vbar
	for (int i = 0; i < 5; i++)
	{
		points[i] = Point(vbar[i][0], vbar[i][1]);
	}
	g->DrawLines(penvbar, points);

	delete points;
	delete g;
	delete pictureBox->Image;
	pictureBox->Image = bitmap;
}

System::Void InertialNavDisplay::drawHeading(PictureBox^ pictureBox, float yaw)
{
	int w = pictureBox->Size.Width;
	int h = pictureBox->Size.Height;
	int headx = (yaw / Math::PI + 1) * w / 2;
	int headay = 1;
	int headby = pictureBox->Size.Height-4;

	Bitmap^ bitmap = gcnew Bitmap(w, h);
	Graphics^ g = Graphics::FromImage(bitmap);
	g->Clear(Color::White);

	{
		int xdiv = 4;
		int xdivref = w / xdiv;
		//vertical ref
		for (int i = 1; i < xdiv; i++)
		{
			int x = xdivref * i;
			g->DrawLine(penref,
				x, 0, x, h);
		}
	}

	g->DrawLine(penheading,
		headx, headay,
		headx, headby
		);

	delete g;
	delete pictureBox->Image;
	pictureBox->Image = bitmap;
}

System::Void InertialNavDisplay::drawCombined(PictureBox^ pictureBox, float pitch, float roll, float yaw)
{
	int vbar[5][2];
	array<Point>^ points = gcnew array<Point>(5);
	int w = pictureBox->Size.Width;
	int h = pictureBox->Size.Height;
	int xcenter = w / 2;
	int ycenter = h / 2;

	int pitchscaled = h * pitch / Math::PI;
	int attx = (yaw / Math::PI + 1) * xcenter;
	int atty = ycenter - pitchscaled;
	vbarpos(vbar, pitch, roll, attx, atty);

	Bitmap^ bitmap = gcnew Bitmap(w, h);
	Graphics^ g = Graphics::FromImage(bitmap);
	g->Clear(Color::LightGray);

	//center dot
	g->FillEllipse(brush,
		attx - dotsize / 2, atty - dotsize / 2,
		dotsize, dotsize
		);

	//vbar
	for (int i = 0; i < 5; i++)
	{
		points[i] = Point(vbar[i][0], vbar[i][1]);
	}
	g->DrawLines(penvbar, points);

	g->DrawEllipse(penpitchref,
		attx - 50, atty - 50,
		100, 100
		);

	delete points;
	delete g;
	delete pictureBox->Image;
	pictureBox->Image = bitmap;
}

System::Void InertialNavDisplay::drawAccelerationXY(System::Windows::Forms::PictureBox ^ pictureBox, float accelx, float accely)
{
	int w = pictureBox->Size.Width;
	int h = pictureBox->Size.Height;
	int xcenter = w / 2;
	int ycenter = h / 2;

	Bitmap^ bitmap = gcnew Bitmap(w, h);
	Graphics^ g = Graphics::FromImage(bitmap);
	g->Clear(Color::White);

	int xdiv = 6;
	int xdivref = w / xdiv;
	int ydiv = 6;
	int ydivref = h / ydiv;
	//ref grid
	{
		//vertical
		for (int i = 1; i < xdiv; i++)
		{
			int x = xdivref * i;
			g->DrawLine(penref,
				x, 0, x, h);
		}
		//horizontal
		for (int i = 1; i < ydiv; i++)
		{
			int y = ydivref * i;
			g->DrawLine(penref,
				0, y, w, y);
		}
	}

	g->DrawLine(penvec,
		xcenter, ycenter,
		(int)(accelx * xdivref) + xcenter, ycenter - (int)(accely * ydivref));

	delete g;
	delete pictureBox->Image;
	pictureBox->Image = bitmap;
}

System::Void InertialNavDisplay::drawAccelerationZVert(System::Windows::Forms::PictureBox ^ pictureBox, float accelz)
{
	int w = pictureBox->Size.Width;
	int h = pictureBox->Size.Height;
	int xcenter = w / 2;
	int ycenter = h / 2;

	Bitmap^ bitmap = gcnew Bitmap(pictureBox->Size.Width, pictureBox->Size.Height);
	Graphics^ g = Graphics::FromImage(bitmap);
	g->Clear(Color::White);

	int xdiv = 6;
	int xdivref = w / xdiv;
	int ydiv = 6;
	int ydivref = h / ydiv;
	//ref grid
	{
		//horizontal
		for (int i = 1; i < ydiv; i++)
		{
			int y = ydivref * i;
			g->DrawLine(penref,
				0, y, w, y);
		}
	}

	g->DrawLine(penvec,
		xcenter, ycenter,
		xcenter, ycenter - (int)(accelz * ydivref));

	delete g;
	delete pictureBox->Image;
	pictureBox->Image = bitmap;
}
