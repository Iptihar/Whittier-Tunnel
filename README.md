# Whittier-Tunnel
Multi thread tunneling using PTHREAD

The purpose is to simulate the operation of the tunnel using semaphores and a shared memory segment.

need to compile using g++ in linux environment

compile: g++ -fpermissive tunnel.cpp -lpthread -o tunnel
run:     ./tunnel input.txt
