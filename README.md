# vtf
A simple vulkan compute shader TF implementation for mersenne primes.  This isn't meant
for production, mostly to explore what can be done and how well in a computer shader.

For vulkan tools see: https://www.lunarg.com/vulkan-sdk/

To compile the CPU side:

`g++ -O -o main main.cpp -lvulkan`

There is a comp.spv SPIR-V binary checked in to git, but to re-compile the shader code into SPIR-V, use either:

`glslangValidator --target-env vulkan1.3 -V tf.comp`

or:

`glslc --target-env=vulkan1.3 tf.comp -o comp.spv`

To run, use something like:

`./main 262359179 3781738011656 3789738911656 0`

The arguments are the exponent to be test, starting and ending K-values, and the vulkan device number to be used.
Note that the startng K values will be reduced to be 0 mod (19399380), so it might jump back a little.

Factors tested are computed by K * 2 * P + 1.  To compute a starting K by factor bit size X, use 2^(X-1) / P.
Using the unix 'dc' calculator, you can do something like:

`2 74 1 - ^ 262359179 / p`

which returns 35999247298068.  Use twice that as the ending value to cover 2^74 - 2^75.
(Feel free to improve the user experience on the CPU side, it is just barely enought to test with currently)

If adding new code in the shader, eventually you will mess up and get it in an infinite loop.  Everything else
on your GPU, including your display will hang.  Give it about 12 seconds, and it should time out and recover.
