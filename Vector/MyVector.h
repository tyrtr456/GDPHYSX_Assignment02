#pragma once

#include <iostream>
#include <cmath>

namespace vectors {
    class MyVector {
    private:
        float x, y, z;

    public:
        MyVector(float x, float y, float z);


    public:
        // Magnitude
        float calculateMagnitude();

        // Direction (unit vector)
        MyVector calculateDirection();

        // Addition
        MyVector addVector(MyVector* add);

        // Subtraction
        MyVector subtrVector(MyVector* sub);

        // Scalar Multiplication
        MyVector scaleMultVector(float scalar);

        // Component Product (Hadamard Product)
        MyVector componentProduct(MyVector* other);

        // Scalar Product (Dot Product)
        float dotProduct(MyVector* other);

        // Vector Product (Cross Product)
        MyVector crossProduct(MyVector* other);

        // Print Vector
        void printCurrentVector();

    public:
        float getX();
        float getY();
        float getZ();
    };
}

