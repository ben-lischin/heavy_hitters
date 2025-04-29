# &phi;-Heavy Hitters
<u>def</u>: Given an input stream S of size N, a φ-heavy hitter is an item that occurs at least φN times in S.

This project explores approximate data structures for frequency estimation in a single-pass stream, offering significant memory reduction compared to traditional hash tables:
1. Count Sketch
2. Count-Min Sketch
3. Misra-Gries

### Running Locally
Install the necessary dependencies: `sudo apt install libssl-dev`

You are provided with the following make commands...

- `make test` (default) - compile test.cpp for evaluating space, accuracy (precision, recall), and time performance for the sketches

- `make clean`

`./test N φ` requires inputs defining stream size `N` and heavy hitter parameter `φ`.
