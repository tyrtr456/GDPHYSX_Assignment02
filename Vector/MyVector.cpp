#include "MyVector.h"

using namespace vectors;

float MyVector::calculateMagnitude() {
    return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}

// Direction (unit vector)
MyVector  MyVector::calculateDirection() {
    float mag = calculateMagnitude();
    return MyVector(this->x / mag, y / mag, z / mag);
}

// Addition
MyVector MyVector::addVector(MyVector* add) {
    return MyVector(this->x + add->getX(), this->y + add->getY(), this->z + add->getZ());
}

// Subtraction
MyVector MyVector::subtrVector(MyVector* sub) {
    return MyVector(this->x - sub->getX(), this->y - sub->getY(), this->z - sub->getZ());
}

// Scalar Multiplication
MyVector MyVector::scaleMultVector(float scalar) {
    return MyVector(this->x * scalar, this->y * scalar, this->z * scalar);
}

// Component Product (Hadamard Product)
MyVector MyVector::componentProduct(MyVector* other) {
    return MyVector(x * other->getX(), y * other->getY(), z * other->getZ());
}

// Scalar Product (Dot Product)
float MyVector::dotProduct(MyVector* other) {
    return x * other->getX() + y * other->getY() + z * other->getZ();
}

// Vector Product (Cross Product)
MyVector MyVector::crossProduct(MyVector* other) {
    return MyVector(
        y * other->getZ() - z * other->getY(),
        z * other->getX() - x * other->getZ(),
        x * other->getY() - y * other->getX()
    );
}

// Print Vector
void MyVector::printCurrentVector() {
    std::cout << "Vector(" << x << ", " << y << ", " << z << ")" << std::endl;
}

float MyVector::getX(){
    return this->x;
}
float MyVector::getY(){
    return this->y;
}
float MyVector::getZ(){
    return this->z;
}