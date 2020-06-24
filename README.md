# PDF Organiser

## Manage your collection of PDF files

PDF Organiser gathers PDF files distributed throughout your system into one place where you can organise them into folders of your choosing. It has minimal dependencies of gtk and json-glib so it is very lightweight and fast with a binary size of less than 45 Kb.<br>


### Download
Precompiled binaries for Linux are available in [rpm](https://github.com/johndoer/pdforganiser/releases/download/0.1/pdforganiser-0.1-1.el7.centos.x86_64.rpm) and [deb](https://github.com/johndoer/pdforganiser/releases/download/0.1/pdforganiser_1.0.0-1_amd64.deb) format on the releases page.


### Installation
```bash
git clone https://github.com/johndoer/pdforganiser.git
cd pdforganiser
./configure
make
make install
```

### Usage
Select PDF Organiser from your Applications menu or type in a console:
```bash
pdforganiser
```
On first load the PDF files list will be empty. Click the **Scan** button at the bottom right corner of the window. A file chooser dialog will appear where you can select the directory you wish to scan. This will recursively scan that directory for any PDF files (Note: if your file system is very large this can take up to a few minutes to complete). At the end of the scan the main window will be populated with all found PDF files. Then you can click **Add Folder** button. A folder icon will appear below all the PDF files - click on it and give it a name of your choosing. You can then drag and drop PDF files of that particular subject into that folder. Click on 'Add Folder' again and create another folder for another subject. You can do this for as many subjects as you like. Folders can be nested inside other folders. When you are done, click **Save Current State** button for your changes to be saved.

