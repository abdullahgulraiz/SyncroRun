#### FreqAndPhase.m

Allows comparison of *Position vs. Time* graph with corresponding *Frequency vs. Time* graph for real-world coordinates of user's face.

##### Input

Reads CSV file containing 3 columns, and equal rows of data, with following format:

| Time (s) | Position X (m) | Position Y (m) |
| -------- | -------------- | -------------- |
| ...      | ...            | ...            |

where

- *Time: time elapsed since start (in seconds)*
- *Position X: x-coordinate of center of bounding box of user's face (in meters)*
- *Position Y: y-coordinate of center of bounding box of user's face (in meters)*