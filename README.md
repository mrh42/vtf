# vtf
simple vulkan compute shader TF implementation

Some design thoughts
- Very minimal data movement between the cpu/gpu. R/W to/from gpu memory is really slow from the cpu. So it writes some tables before the first call, but after that the cpu only looks at a couple uint64_t values per call.

- Try to keep all the gpu "threads" doing useful work. I found my gpu likes a lot. Like 512k threads. So be very mindful of branching, etc. You don't want one edge case going while 1000s are waiting for it to catch up.

- To keep everyone working on something, requires a little memory trade-off. So each thread starts with a range of 200ish K values, and builds an array of 30ish to pass to the next sieve stage. Something like 13 stages of list shortening gave good results. I think this can still be improved, the first stage should be broken up.

- This pre-work is done with 32bit P and 64bit K values, but it still does a number of integer mod operations. So I found using around 200 small primes for the sieve() was a good balance for saving work in the next step.

- For the K values that make it through the sieve, we finally create Q from P*2*K+1, start squaring. This will perform 10*log2(Q) * log2(P) - ish simple operations on a 96bit extended uints. These are fast, but that is a lot of operations, 20K maybe? I'll work on this next.

That last step works well across the threads, but there is still some variation in the length of the lists of k-values to test between threads. So there are some threads waiting around, that could be doing work. I don't know what to do about that, maybe threads within a work group could balance out the work lists. There are some mechanisms for that.
