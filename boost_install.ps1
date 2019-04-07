wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.zip -O boost.zip
mkdir Library/boost
Expand-Archive -Path boost.zip -DestinationPath Library/boost
Move-Item Library/boost/boost_1_69_0/* Library/boost/
rmdir Library/boost/boost_1_69_0
cd Library/boost
./bootstrap.bat
./b2.exe --with-serialization
rm ../../boost.zip