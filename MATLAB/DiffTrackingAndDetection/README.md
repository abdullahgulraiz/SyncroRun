#### DiffTrackingAndDetection.m

Allows comparison of *Position vs. Time* graphs for data collected from Face Tracking (Haar cascade classifier) and Face Detection on a per-frame basis. The comparison helped decide between using tracking by detection, or just using tracking initialized by detection.

##### Input

Reads CSV file containing 3 columns, and equal rows of data, with following format:

| Time (s) | Position X (m) | Position Y (m) |
| -------- | -------------- | -------------- |
| ...      | ...            | ...            |

where

- *Time: time elapsed since start (in seconds)*
- *Position X: x-coordinate of center of bounding box of user's face (in meters)*
- *Position Y: y-coordinate of center of bounding box of user's face (in meters)*