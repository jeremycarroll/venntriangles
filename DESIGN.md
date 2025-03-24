
## Guidance about Algorithm And Code Layout


| Geometric Concept  | File | Notes |
| ------------- | ------------- | ---- |
| Color | color.c | Edge Label |
| ColorSet | color.c | Face Label |
| Cycle | color.c | Sequence of Edges around a Face |
| CycleSet | color.c | Possible Sequences of Edges around a Face |
| Undirected Point | point.c | A possible oriented point between 4 specific faces |
| Directed Point | point.c | A point on a labelled curve |
| Edge | edge.c | A directed, labelled side of a face, between two points |
| Curve | edge.c | a connected sequence of edges with the same label |
| Triangle | edge.c | a closed curve, satisfying various triangle rules in relationship to other triangles |
| Face  | face.c  | A face of a Venn diagram |