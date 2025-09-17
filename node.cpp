#include "node.h"
#include <limits>
#include <cmath>
#include <fstream>

// Qt JSON includes
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

// --- Constructors ---
Node::Node()
    : x(0.0), y(0.0), value(std::numeric_limits<double>::quiet_NaN()) {}

Node::Node(double x_, double y_, double value_)
    : x(x_), y(y_), value(value_) {}

Node::Node(const Node& other)
    : x(other.x), y(other.y), value(other.value) {}

Node::Node(Node&& other) noexcept
    : x(other.x), y(other.y), value(other.value) {}

// --- Assignment operators ---
Node& Node::operator=(const Node& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        value = other.value;
    }
    return *this;
}

Node& Node::operator=(Node&& other) noexcept {
    if (this != &other) {
        x = other.x;
        y = other.y;
        value = other.value;
    }
    return *this;
}

// --- Comparison ---
bool Node::operator==(const Node& other) const {
    return (x == other.x && y == other.y && value == other.value);
}

bool Node::operator!=(const Node& other) const {
    return !(*this == other);
}

// --- Append / update ---
void Node::update(double newX, double newY, double newValue) {
    x = newX;
    y = newY;
    value = newValue;
}

// --- Output ---
std::ostream& operator<<(std::ostream& os, const Node& node) {
    os << "Node(" << node.x << ", " << node.y << ", " << node.value << ")";
    return os;
}

