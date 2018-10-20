
Press 'a' key to start animation
Press 's' key to switch rotation mode

program arguments:

	-l list filters
	-w window width
	-h window height
	-W resized video width
	-H resized video height
	-f starting filter index
	-i filename
	-c web camera mode
	-v video file mode
	-S rotation mode spin
	-C rotation mode cube 
	-s rotation speed
	-u subfilter
	-b draw background

examples

	$ ./acid_cube -w 1280 -h 720 -i filename.mp4 -v

or

	$ ./acid_cube -w 640 -h 360 -c -f 452

or
	$ ./acid_cube -w 1280 -h 720 -v -i filename.mp4 -s 0.9

or
	$ ./acid_cube -l



Up/Down arrow keys to change rotation mode
Left/Right arrow keys to toggle between filters

To compie requires macOS, SDL2,GLEW,OpenCV, and libacidcam
