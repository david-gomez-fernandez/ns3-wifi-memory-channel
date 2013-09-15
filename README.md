****Wireless error models for ns-3****
****Authors: David Gómez Fernández and Ramón Agüero Calvo (dgomez,ramon@tlmat.unican.es)****
****April 2012****

This work includes the implementation of three rather different error-models (designed after the experimental characterization of an indoor wireless channel, namely by means of the
IEEE 802.11b physical extension at its maximum capacity, that is to say, 11 Mbps), whose main features are briefly depicted below:
- BEAR model (acronym of Bursty Error Auto-Regressive Filter). In few words, with this one we estimate the Signal to Noise Ratio (SNR) of a received frame, introducing a memory component
  (through an AR filter). Once calculated the link quality of a concrete frame, it will go through a decider entity which will be in charge of deciding whether the frame is correct or not 
  (according to a logistic function based on the results obtained empirically).
- HMM model (based on a Hidden Markov Process). In this case we have started from a completely different approach: we have processed the trace files obtained over the real measurement
  campaign (and we have observed a huge level of variability among the results, yielding some cases with a scarce error presence and, on the other hand, transmissions highly damaged due to
  losses over the wireless link), and trained (through Matlab's "hmmtrain" function) them in order to get a set of transition/decision matrices able to mimic the behaviour of the studied
  measurements. One of the main distinctive points of this model (this could be considered a drawback as well) is that it does not have any kind of dependency with the distance between the 
  nodes, leaving its total character to the matrices that mark the system behaviour.
- Simple propagation loss model. Finally, we carried out a last and simlple propagation model which acts as a binary decisor of the correctness of a frame, according to the distance between the nodes;
  in other words, it will showcase three regions: the first one will lead to a situation where every frame gets lost (it will yield -10000 dB as a result at the reception side); the second 
  one will show a decreasing shape (following a function included at the code), starting from a total loss situation, until a point from which all frames will be received correctly. For the cases where a frame is not corrupted, we will
  keep the frame's power value at the transmitter node, assuring that the receiver will not discard it. 

This repository contains the legacy ns-3.13 "folder" with the modifications we have carried out within. You can find new folders (e.g. src/ar-model and hidden-markov-error-model) 
and small modifications in some of the files as well (e.g. src/wifi/model/yans-wifi-phy.*, src/wifi/helper/yans-wifi-phy-helper.*). It is worth highlighting the addition of
another naive propagation loss model (a.k.a. SimplePropagationLossModel), integrated in the file "src/propagation/model/propagation-loss-model.*", but its relevance is negligible in 
comparison with the other ones. 

There is an example to test our implementation (and another one chosen from the set of default propagation and error models). This one corresponds with a simple scenario where a node 
sends "N" packets of "L" bytes length (values chosen by the user) with a gap of "T" seconds between them (at application layer), to a sink node separated a distance of d meters.

You can find it in the file "scratch/error-model.test". Here is a sneak-peek of the options we can tweak:
 - Channel model: we can choose among BEAR, HMM or a combination of LogDistancePropagationLossModel + YansErrorRateModel (these ones could be found in the legacy source code).
 - Transport protocol: This is really simple: TCP or UDP.
 - Distance: As could be easilly inferred, this parameter sets the distance between the source and the sink nodes. It is worth highlighting that
 - Number of packets: The total of packets sent by the application layer at the transmitter node
 - Packet length: For the sake of simplicity, the whole set of packets sent will keep the same length
 - Time interval between packets: With this parameter

Last, but not least, you can find included the patch file "error-models.patch", with which you can directly update your legacy "src" folder (Further details in the manuals included in the
repository).	

NOTE: This repository only includes the "ns-3.13" folder, hence you will need to merge it within the whole framework to make it work.