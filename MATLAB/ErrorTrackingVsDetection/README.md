#### ErrorTrackingVsDetection.m

Helps compare difference in positions of bounding box centers of user's face when tracked through *Tracking by Detection* and *Tracking initialized by Detection*. 

##### Input

Reads two CSV files containing 3 columns, and equal rows of data, with following format:

| Position X (m) | Position Y (m) | Time (s) |
| -------------- | -------------- | -------- |
| ...            | ...            | ...      |

where

- *Position X: x-coordinate of center of bounding box of user's face (in meters)*
- *Position Y: y-coordinate of center of bounding box of user's face (in meters)*
- *Time: time elapsed since start (in seconds)*

One file contains data from Tracking by Detection, whereas the other contains data from Tracking initialized by Detection.