#include "node.h"

/**
 * @class StreamNetwork
 * @brief Represents a directed stream network built from Node objects.
 */
class StreamNetwork {
public:
    using Edge = std::pair<int,int>; // (fromIndex -> toIndex)

    StreamNetwork() = default;

    /// Construct from nodes
    explicit StreamNetwork(const std::vector<Node>& nodes);

    /// Build directed network from nodes (higher value -> lower value)
    static StreamNetwork buildDirected(const std::vector<Node>& nodes);

    /// Add an edge
    void addEdge(int from, int to);

    /// Getters
    const std::vector<Node>& nodes() const;
    const std::vector<Edge>& edges() const;
    int nodeCount() const;
    int edgeCount() const;

    void printSummary(std::ostream& os = std::cout) const;

    /**
     * @brief Save only the edges of the stream network as GeoJSON LineStrings.
     *
     * @param fileName Output filename (e.g. "network_edges.geojson").
     */
    void saveEdgesAsGeoJSON(const QString& fileName) const;


private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
};
