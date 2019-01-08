#ifndef PATH_HPP
#define PATH_HPP

#include <OverlapGraph.hpp>
#include <climits>

class Path {
private:
    long length_;
public:
    std::vector<const OverlapGraph::Node *> nodes_;
    std::vector<const OverlapGraph::Edge *> edges_;

    void updateLength();

    long length() const;

    friend std::ostream &operator<<(std::ostream &s, const Path &p);

    bool operator==(const Path &other) const;
};

#endif
