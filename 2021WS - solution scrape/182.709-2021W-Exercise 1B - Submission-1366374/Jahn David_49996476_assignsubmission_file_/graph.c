/**
 * @file graph.c
 * @author David Jahn, 12020634
 * @brief The source file and implementation of the graph datatype
 * @version 1.0
 * @date 2021-11-14
 * 
 */
#include "graph.h"

void color(graph_t *graph, int color, int node)
{
    node_t tmpNode;
    for (int i = 0; i < graph->edges_len; i++)
    {
        tmpNode = graph->edges[i].to;
        if (tmpNode.id == node)
            tmpNode.color = color;
        tmpNode = graph->edges[i].from;
        if (tmpNode.id == node)
            tmpNode.color = color;
    }
}

bool isColored(graph_t *graph, int node)
{
    bool ret = false;
    node_t tmpNode;
    for (int i = 0; i < graph->edges_len; i++)
    {
        tmpNode = graph->edges[i].to;
        if (tmpNode.id == node)
            return tmpNode.isColored;
        tmpNode = graph->edges[i].from;
        if (tmpNode.id == node)
            return tmpNode.isColored;
    }
    return ret;
}

bool nodeExists(graph_t *graph, int nodeId)
{
    bool ret = false;
    node_t tmpNode;
    for (int i = 0; i < graph->edges_len; i++)
    {
        tmpNode = graph->edges[i].to;
        if (tmpNode.id == nodeId)
            return true;
        tmpNode = graph->edges[i].from;
        if (tmpNode.id == nodeId)
            return true;
    }
    return ret;
}

graph_t removeEdge(graph_t graph, edge_t edge)
{
    bool shift = false;
    for (int i = 0; i < graph.edges_len; i++)
    {
        if (shift)
        {
            graph.edges[i] = graph.edges[i + 1];
        }
        else if (graph.edges[i].from.id == edge.from.id && graph.edges[i].to.id == edge.to.id)
        {
            graph.edges[i] = graph.edges[i + 1];
            shift = true;
        }
    }
    if (shift)
        graph.edges_len--;

    return graph;
}

node_t getNode(graph_t *graph, int nodeId)
{
    node_t ret;
    node_t tmpNode;
    for (int i = 0; i < graph->edges_len; i++)
    {
        tmpNode = graph->edges[i].to;
        if (tmpNode.id == nodeId)
            return tmpNode;
        tmpNode = graph->edges[i].from;
        if (tmpNode.id == nodeId)
            return tmpNode;
    }
    return ret;
}