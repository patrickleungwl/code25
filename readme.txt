# conda activate py10

sudo apt-get install python3.11-venv
sudo apt-get install cmake
sudo apt-get install libgtest-dev
sudo apt-get install libpthread-stubs0-dev
sudo apt-get install googletest
sudo apt-get install mkdocs
sudo apt-get install python3-pip
pip3 install mkdocs-material
python3 -m venv /home/patrickleungwl/py311
/home/patrickleungwl/py311/bin/pip3 install mkdocs-material

# to run local webserver
mkdocs serve

# to generate site
mkdocs build

# on 60key keyboard
# tilde is fn-shift-esc at the same time

