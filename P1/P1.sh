#!/bin/bash

echo "-We generate the a.txt.crc file from the original a.txt file and verify that a.txt has not been corrupted"

gcc main.c crc.c timer.c -o checksum
./checksum a.txt -generate
./checksum a.txt -verify

echo "-Corruption process"

gcc corrupt.c -o corrupt
./corrupt a.txt -o a_corrupted.txt -numCorruptions 30

echo "-We change the name of a.txt to a_nocorrupted.txt, and the name of a_corrupted.txt to a.txt, to be able to detect the corruption using the original checksum of a.txt"

mv a.txt a_nocorrupted.txt
mv a_corrupted.txt a.txt

./checksum a.txt -verify

echo "-Verify again but setting the maximum number of errors to a very large number"

./checksum a.txt -verify -maxNumErrors 100

echo "-Verify again but setting the maximum number of errors to a very small number"

./checksum a.txt -verify -maxNumErrors 1

echo "-Change the name of the files to the original names so that we can clearly identify which one has been corrupted"

mv a.txt a_corrupted.txt
mv a_nocorrupted.txt a.txt

echo "-Repeat the same process with a large file called large_file.txt"

./checksum large_file.txt -generate
./checksum large_file.txt -verify

echo "-Corruption process"

./corrupt large_file.txt -o large_file_corrupted.txt -numCorruptions 200

mv large_file.txt large_file_nocorrupted.txt
mv large_file_corrupted.txt large_file.txt

echo "-Verify with the corrupted large file and reverse change of names"

./checksum large_file.txt -verify

mv large_file.txt large_file_corrupted.txt
mv large_file_nocorrupted.txt large_file.txt
