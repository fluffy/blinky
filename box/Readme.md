# Box Info

The main design is in the .FCStd files and crearted with FreeCad.

Most the design paramters are on the "Data" sheet.

Select the object in FreeCad to export the parts as .3mf and .step. The
exported files are in the prod subdirectory.

Go to the front and back drawing and right click on page to select
"Export DXF" and export as .dxf to the prod subdirectory. Using the
export option on the file menu does not work. The eDrawings app on osx
can look at DXF and STEP files.

Import the DXF files into XCR or lightburn for laser cutting.

Upload the step file to sendcutsend.com or jlcpcb.co. With send cut
send, had selected 2mm thick 6061 T6 Aluminum and had it annodized.

3D print using the .3mf files. Set a inner brim of 7 mm on 1st layer
seems to help with small holes.


## Data Flow

KiCad generates step models for the PCBs and puts them in  hardware/blink/productions dir.

This PCB is imported into the FreeCAD drawing for box which generates DXF files in box/prod dir.

This is brought in to laserA.xcs with "xTool Creative Space" application
to add the labels and set up for laser cutting.
