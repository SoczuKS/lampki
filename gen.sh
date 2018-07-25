cd res

g++ gen.cpp -o hexdumper.bin
tr -d '\r\n\t' < control3.html > index.html
pandoc --from markdown_github --to html5 ../README.md -o readme.html
{
echo '#ifndef _RESOURCES_H'
echo '#define _RESOURCES_H'
echo 'struct Resources {'
./hexdumper.bin index.html index_html
./hexdumper.bin readme.html readme_html
./hexdumper.bin help.txt help
./hexdumper.bin ../README.md readme_src
./hexdumper.bin moreleled.service moreleled_service
./hexdumper.bin favicon.ico favicon_ico
./hexdumper.bin favicon.png favicon_png
./hexdumper.bin pinout.png pinout_png
echo '};'
echo '#endif'
} > ../src/Resources.h

cd ..

#convert favicon.ico favicon.png

