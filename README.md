# ReferenceBeamDetection

Segmentation of a reference visible laser beam targeting augmentation of fluorescence lifetime maps on white light images. 


## Description

In the context of fibre-based single point fluorescence measurements, one can use excitation light in the visible (or a second visible laser beam in case of UV 
excitation) together with a camera pointing at the sample to provide a visual reference for the exact location of the measurements, thereby adding spatial resolution 
to single point measurements. Specifically, this project provides the code to detect a reference beam and overlay the segmented area on the original white light image.
The segmented region can be color coded according to the measured fluorescence parameter (e.g. fluorescence lifetime) to realize augmented fluorescence measurements.
Current implementation is targeted for an IR beam, although it can be easily modified to segment profiles of different wavelengths.


This project provides the core code underlying a DLL to facilitate deployment with other applications and/or softwares.


Current version tested with LabVIEW 2015 (64 bit) & NI VISION toolkit.


## Requirements

- C++ editor and compiler (suggest using Visual Studio Community)

- opencv-2.4.13 (other versions may work but were not tested)


## Interface

### Inputs

- b,g,r
- map counts
- map lifetime
- lifetime
- min_thres
- max_thres
- input radius

### Outputs

- b, g,r
- map counts
- map lifetime
- seg x
- seg y
- radius segmentatio


## To do

- pass image width and height as arguments
- change name of lifetime associated variables to reflect a more general input parameter