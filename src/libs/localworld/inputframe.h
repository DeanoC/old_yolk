#ifndef YOLK_LOCALWORLD_INPUT_FRAME
#define YOLK_LOCALWORLD_INPUT_FRAME 1

struct InputFrame {
	static const int MAX_PADS = 4;

	struct PAD {
		/// movement along the X axis.
		float XAxisMovement;
		/// movement along the Y axis.
		float YAxisMovement;

		/// Button 1 is down.
		bool button1;
		/// Button 2 is down.
		bool button2;
		/// Button 3 is down.
		bool button3;
		/// Button 4 is down.
		bool button4;

		bool debugButton1;
		bool debugButton2;
		bool debugButton3;
		bool debugButton4;
		bool debugButton5;
	} pad[ MAX_PADS ];

	/// position of a mouse X axis.
	float mouseX;
	/// position of a mouns Y axis.
	float mouseY;
	/// Left Mouse Button is down.
	bool mouseLeftButton;
	/// Right Mouse Button is down.
	bool mouseRightButton;

	// absolute mouse X position
	float absoluteMouseX;
	// absolute mouse Y position
	float absoluteMouseY;

	float deltaTime; // time since last input frame
};

#endif
