# ESP32-CAM Config Comparison
The ESP32-CAM does not capture images instantly. The time taken to capture an image varies depending on a few factors: image resolution, JPEG quality and frame buffer.

This project aims to run a simple test to compare photo capture performance across various ESP32-CAM configurations, enabling you to optimize the settings for specific IoT applications. For example, a security mechanism would require rapid, successive snapshots to capture an intruder, while a time-lapse of a growing plant could prioritize image quality over capture speed.

All average readings are based on the average of five successive captures.
## Resolution/Frame Size
|Resolution|Average capture time|File size|
|----------|--------------------|---------|
|1600 x 1200 (UXGA)|||
|320 x 240 (QVGA)|||
|352 x 288 (CIF)|||
|640 x 480 (VGA)|||
|800 x 600 (SVGA)|||
|1024 x 768 (XGA)|||
|1280 x 1024 (SXGA)|||

## JPEG Quality
|JPEG Quality (0-63)|Average capture time|Size|
|-------------------|--------------------|----|
|10|||
|20|||
|30|||
|40|||
|50|||
|60|||


> Written with [StackEdit](https://stackedit.io/).