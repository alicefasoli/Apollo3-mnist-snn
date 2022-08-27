# Spiking-Neural-Networks

This code is designed to demo the use of a Spiking Neural Network (SNN) on the Apollo3 Blue Plus board. 

The network is learned in an unsupervised way, more specifically using the Spike-Time Dependent Plasticity (STDP) to update the weights of the synapses.
The network is trained using the MNIST dataset and achieves an accuracy of 60% with a total of 100 neurons in the second layer.
To test the classification on the microcontroller, we had to give up some accuracy because of memory problems.

## Training

To train the network is necessary to run the file contained in the mnist_snn directory called main.c, this will recreate the network_values.h file and rewrite the weights files. 
You can change some parameters in parameters.h file to see how accuracy changes and how STDP works.

## Testing
To test the network we can do it in two ways:
On pc running the file contained in the mnist_snn directory called classify.c
On the microcontroller, to do this we have to:
the default actual image is the number 0, if you want to test the network on another image you have to run the file create_new_image.c and pass to the function create a random number from 0 to 10000
install the software environment: MicroVision, MDK5, and JLink are required
download the code contained in the directory apollo3_mnist_snn 
build the image 
load the built image on the board
In the second case, we can see that some LEDs are going to turn on. If the classification is correct the LEDs corresponding to the image are going to turn on in binary counting. Otherwise, just the first LED is going to turn on. 
0 00000
1 00001
2 00010
3 00011
4 00100
5 00101
6 00110
7 00111
8 01000
9 01001
Wrong digit 10000
