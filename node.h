#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <QString>

/**
 * @class Node
 * @brief Represents a point in 2D space with an associated value.
 */
class Node {
public:
    double x;      ///< X coordinate
    double y;      ///< Y coordinate
    double value;  ///< Associated value (e.g., elevation)

    // --- Constructors ---
    Node();                                  ///< Default (0,0,NaN)
    Node(double x, double y, double value);  ///< Full constructor
    Node(const Node& other);                 ///< Copy constructor
    Node(Node&& other) noexcept;             ///< Move constructor

    // --- Assignment operators ---
    Node& operator=(const Node& other);      ///< Copy assignment
    Node& operator=(Node&& other) noexcept;  ///< Move assignment

    // --- Comparison operators (by value, then x,y) ---
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    // --- Append-style update ---
    void update(double newX, double newY, double newValue);

    // --- Output helper ---
    friend std::ostream& operator<<(std::ostream& os, const Node& node);

    // --- Output helper ---
    friend std::ostream& operator<<(std::ostream& os, const Node& node);

};

#endif // NODE_H
