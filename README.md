# ESP32-CAM Config Comparison
The ESP32-CAM does not capture images instantly. The time taken to capture an image varies depending on a few factors: image resolution, JPEG quality and frame buffer.

This project aims to run a simple test to compare photo capture performance across various ESP32-CAM configurations, enabling you to optimize the settings for specific IoT applications.

For example, a security mechanism would require rapid, successive snapshots to capture an intruder, while a time-lapse of a growing plant could prioritize image quality over capture speed.

All average readings are based on the average of five successive captures, with a JPEG quality value of 10 (explained below).
## Resolution/Frame Size
*Arranged from highest to lowest resolution*
|Resolution|Average capture time (s)|File size (bytes)|
|----------|--------------------|---------|
|1600 x 1200 (UXGA)|3.23|99637|
|1280 x 1024 (SXGA)|1.31|38610|
|1024 x 768 (XGA)|0.70|23756|
|800 x 600 (SVGA)|0.65|20027|
|640 x 480 (VGA)|0.29|9845|
|352 x 288 (CIF)|0.17|5625|
|320 x 240 (QVGA)|0.17|4009|
A similar test can be conducted with different values of JPEG quality.

JPEG quality can be varied from 0 - 63, with 0 being the highest quality, 63 being lowest, in the setup configuration.

> Written with [StackEdit](https://stackedit.io/).
