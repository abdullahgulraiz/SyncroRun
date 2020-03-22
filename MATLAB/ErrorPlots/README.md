#### CenterAndBounding.m

Plots graphs of *Bounding Box Overlap Ratios vs. Frames* and *Center Location Error vs. Frames*.

#### Input

***BoundingBoxes.csv***

CSV file containing 4 columns, and equal rows of data, with following format:

| Time | X    | Y    | W    | H    |
| ---- | ---- | ---- | ---- | ---- |
| ...  | ...  | ...  | ...  | ...  |

where

- *Time: time elapsed since start (in seconds)*
- *X: x-coordinate of position of bounding box corner of user's face (in pixels)*
- *Y: y-coordinate of position of bounding box corner of user's face (in pixels)*
- *W: width of bounding box of user's face (in pixels)*
- *H: height of bounding box of user's face (in pixels)*

***CenterLocationError.csv***

CSV file containing 2 columns, and equal rows of data, with following format:

| Time | Error |
| ---- | ----- |
| ...  | ...   |

where

- *Time: time elapsed since start (in seconds)*
- *Error: error between center locations of tracked bounding box and actual bounding box of user's face (in pixels)*

