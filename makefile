#Ben and Anthony

trapezoid: trapezoid.cpp childData.h child
	c++11 trapezoid.cpp -o trapezoid

child: callFunction.cpp childData.h 
	c++11 callFunction.cpp -o child