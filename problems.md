# Problems with Implementing an Assembler for the VM
Well, I have made an attempt in the past by building **Zeta** to make it easy to write programs for Merry. The attempt may have been successful but a single person can only get so far.
I couldn't keep up with the features that I planned and my pace of implementing them. As a result, **Zeta** was a mess and a failure in my opinion even though it did what is was supposed to
do. This new attempt is the same. I made a working prototype in Python that isn't hosted but now that I start implementing in C++, my mind floods with features that I want to add and I 
just can't keep up with the mess. 

# Expectations VS Reality
- The truth is that I really like C's error messages. C++'s error messages are like reading alien language for most of the time thanks to the clever names used and the template hell. My goal has always been to implement that level of error handling which is simple in words but hard to put into code. The decisions I make midway as I code really mess up what I was previously doing and in the end I, myself, becomes unable to tell what part is doing what. 
- I want to add expression parsers for all kind of expressions but, again, with my old experience with building expression parsers I know how hard it will be to integrate one into the assembler.
- I want to make it easier to program using the assembler by providing a lot of syntactic sugar around the underlying instructions basically being like PHP i.e forgiving. For eg: If a
user writes "mov Ma, 12" and also "mov Ma, Mb", the assembler will handle figuring out which underlying instructions should be used to accomplish that. This is easier than writing
"mov_regr_imm Ma, 12" and "mov_regr_regr Ma, Mb", isn't it? This was actually implemented in **Zeta** but it was a lot of work.
- I want the assembler to allow for expressions and syntax much similar to a high-level language thus keeping it low-level while also providing a significant base to work using higher level constructs.
- The plan with this attempt was to make sure that this assembler assembled to actual machine code as well without using LLVM. 

# I give up
I give up writing an assembler for now. Maybe I will change the codegen for **Zeta**? Using that crap isn't the best of choice though.