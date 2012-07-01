#!/usr/bin/python

from __future__ import division;
import os;
import math;


disk_size = 5120000;

stage1_size = os.path.getsize("boot/stage1");
stage2_size = os.path.getsize("boot/stage2");
pad_size = os.path.getsize("boot/pad");
kernel_size = os.path.getsize("obj/kern/kernel");

print "stage1 size: %d" % stage1_size;
print "stage2 size: %d" % stage2_size;
print "pad size: %d" % pad_size;
print "kernel size: %d" % kernel_size;

post_pad_size = disk_size - stage1_size - stage2_size - pad_size - kernel_size;

print "will generate a post_pad of size: %d" % post_pad_size;

dd_cmd = "dd if=/dev/zero of=boot/post_pad bs=%d count=1" % post_pad_size;

os.system(dd_cmd);

kernel_start = math.ceil((stage1_size + stage2_size + pad_size)/512);
kernel_len = math.ceil(kernel_size/512);

print "GRUB load command: kernel %d+%d" % (kernel_start, kernel_len);
