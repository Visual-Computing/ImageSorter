ImageSorter 4.3 Beta

Copyright Visual Computing Group at the HTW Berlin

www.visual-computing.com

(C) 2005 - 2023
**********************

ImageSorter is an image browsing application, which allows automatics sorting of images by visual similarity. All images within one or more folders are sorted in such a way, that similar images are positioned close to each other. This sorting scheme makes it much easier to find a particular image within a huge set of images compared to other approaches used today that mostly can do a sorting by name, date or file size only. The sorting results of ImageSorter are more impressive for larger image sets (a folder containing 1000 images compared to a folder containing only 10 images).


0. Changes
**********

ImageSorter V4.3 Beta contains a brand new sort engine, which is much faster than the previous one used in ImageSorter v4.2. 

ImageSorter V4.3 does not allow to download images from yahoo or flickr any longer. Please note that the yahoo Images Search API is not longer available. Please note that from the release date of ImageSorter V4.3 Beta on, also older versions like ImageSorter V4.2 do not support downloading images from flickr any longer (because the flickr API key is disabled).

1. Loading and sorting images
*****************************

Use the explorer tree widget in the upper left to navigate to a folder containing images. Select more folders by pressing Ctrl or Shift while clicking the explorer tree widget.

By clicking the 'Laod and sort' button ImageSorter will load the images and sort these images according to their visual similarity. ImageSorter will cache thumbnails of the loaded images. Thus loading images will be much faster if the same folder(s) are displayed and sorted again (as long as the content of the folder(s) hasn't changed in the meantime).

If you select a folder without images, the main widget will show the message: 'Selected Folder does not contain images'.

If you select the 'Load subdirectories' option from the 'Options' menu, ImageSorter will load all subfolders of a folder, too.

If you press 'ESC' during the sorting process, sorting will be aborted. The current sorting stage will be displayed. If you press 'ESC' while the images are loaded, loading is aborted.

Please note the special nature of the visually sorted display. The display area is wraped on its sides and you can drag it by pressing the left mouse button (on a region where no image is displayed) and move the mouse. Images that are dragged over the right edge of the main image will re-appear at the left edge, Images that are dragged over the top of the main image will re-appear at the bottom. In general, you may find it useful to drag the part of the map, which you are interested in to the middle of the display area.

Sort Options:
The spacing of the sorted images may be adjusted. A value of 0 will result in a very dense packing of the images with very few gaps in the visually sorted image map. A larger image spacing value generally will lead to a better visual sorting result, however because of the extra gaps the thumbnails need to be smaller. For too large values of the image spacing parameter the sorting might appear irregular. Please note that you need to resort the images (using the right mouse button context menu) after you have changed the image spacing value. ImageSorter uses a default image spacing value of 2.
 

2. Finding similar images on your computer
******************************************

Switch the 'Source' drop box in the top right to 'Harddisk'. Switch the 'Visual Filters' panel on the right to 'Example Images'. Drag one or more images from a given set of images from the main area to the area below 'Example Images' and click 'Search Images'. You will be asked to select one or more folders from the explorer window at the left. When done, click the 'Search Images' button in the center of ImageSorter. The folder(s) will be loaded and those images that are most similar to the example images will be shown. Note that the view is updated as long images are loaded and that images displayed may disappear again if better matches are found. You can delete images in the 'Example Images' area by pressing the 'x' in the top right of the images. You can delete all images by pressing the 'x' right from 'Visual Filters'.

You may search for a self-drawn sketch by switching the 'Visual Filters' to 'Sketch'. Select one of the three round shapes and one or more colors, draw your sketch and press the 'Search Images' button. You can delete the sketch by pressing the 'x' right from 'Visual Filters'.

You may also search for images containing one ore more colors by switching the 'Visual Filters' to 'Color'. Select up to four colors from the color panel and press the 'Search Images' button. You can delete colors by pressing the 'x' right of the color. You can delete all colors by pressing the 'x' right from 'Visual Filters'.

You can also filter the hard disk search by entering a filename in the 'Filename' text entry at the top right. Note that wildcards like '*.jpg' are not supported.

You may also use 'File Filters' at the right. Choose a file type, the size, the coloration and/or the orientation.


3. Changing the image sorting mode
**********************************

Images may be sorted by color, by name, by size or - if possible - by date or by similarity. Use the five topmost buttons between the main area and the left explorer window or the 'View' menu for this.


4. Selecting images
*******************

When clicking an image with the left mouse button, it will be selected or deselected. If selected, it will be displayed in the 'Preview' widget in the bottom left. A double-click on an image with the left mouse button will launch the application which generally opens the file type on your system - your browser in case the image was loaded from the internet or your standard application for image types if the image was loaded from disk.
Note this may take some time, but you can continue to use ImageSorter.

You may select more images by pressing ‘Shift’ and dragging the mouse while the left button is pressed. All images inside the rectangular region, which is displayed, will be selected.

You can deselect images by pressing ‘Alt’ + ‘Shift’ and by dragging the mouse while the left button is pressed. All selected images inside the drawn rectangular region will be deselected.


5. Zooming
**********

You can zoom in the main window by either using the mouse wheel or by the zoom buttons or by the 'View' menu. Note that the main window shows thumbnails of the original images but loads and displays the original image (if loaded from disk) if the zoom factor is high enough.


6. Context menu
***************

Pressing the right mouse button in the main window displays a context menu.


7. Resorting the images
***********************

You can resort the loaded images using the context menu. This is particularly useful if you have resized ImageSorter - in this case the old sorting may look bad. Note that as opposed to former versions, ImageSorter 4.3 Beta can sort maps to any aspect ratio. Prior versions always sorted on to a square map.


8. Copy, Move and Delete selected Images
*****************************************

You may copy, move or delete selected images by the context menu. Note that you cannot copy or move an image if the destination image exists or if the destination is one of the folder(s) currently loaded. Images that are moved or deleted are not shown any longer in the main window. You can resort the remaining set by choosing 'Resort' from the Context menu.

Note that if the 'Resort on Hide, Move and Delete' option (in the Options menu) is on, moving or deleting images will resort automatically ('hiding images' is no longer supported).


9. Copy to clipboard
*********************

You can copy the selected image (if loaded from disk) to the clipboard using the context menu. Note that this only works if one and only one image is selected.


10. Image formats and loading errors
************************************

ImageSorter can read images in jpeg, tiff, png, gif, bmp, pbm, pgm, ppm, xbm, and xpm formats. However, if an image of these types cannot be read, the main window will show up a question mark icon instead. In this case, the image has a bad format.