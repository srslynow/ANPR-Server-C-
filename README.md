# ANPR-Server-C++
A multithreaded Automatic Number Plate Recognition (ANPR) server written in C++, much like [OpenALPR](https://github.com/openalpr/openalpr).

A few features
- Communication with a client source (webcam/security cam/image/video) happens over UDP instead of TCP
 - This provides us with much faster processing & lower latency (e.g. ~2-3 ms instead of 100 ms delay on a local network)
 - Some example scripts how to provide data to this server are in my ANPR-Client-Python repository
- Program is completely multi-threaded (frame based) and scales up to hundreds of cores.
- Uses Deep Neural Networks (DNNs) for
 - Segmentation: masking & isolating characters in a licence plate image
 - Classification: classification of characters in a licence plate, e.g. say that an image contains a '7'
- Uses Histogram of Gradients (HoG) descriptors for licence plate detection

TODOs
- Use DNNs for licence plate localization
- Write sequence id format for the UDP server for the case packets arrive out of order, those frames are dropped now
