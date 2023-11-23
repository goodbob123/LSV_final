# Some common commands 

- &fx : extract shared logic using the classical "fast_extract" algorithm  (reduce Final Literals in SOP,paper in 1992),
 Detects logic sharing by extracting two-cube divisors and two-literal single-cube divisors using the algorithm by Rajski/Vasudevamurthi (TCAD’92)
- &transduction : repeat remove redundant signal using permissible functions (很多種選項)
- &dc2 : performs combinational AIG optimization (合併多種 rw -l; rf -l; b -l; rw -l; rwz -l; b -l; rfz -l; rwz -l; b -l) , also  restructure circuit
- &dc3 : (&jf ; performs technology mapping of the network)  "& b; &jf -K 6; & b; &jf -K 4; &b"
- &dc4 :  "&b; &jf -K 7; &fx; &b; &jf -K 5; &fx; &b"
- &deepsyn : 
- &get : 轉成Gia


todo : 
- transduction 細節
- dc2 細節 , abc.rc中還有dc3,dc4...
- 看LSV的don't care有沒有機會implement
- Abc_CommandRecAdd3 又是啥
- abc.rc 中 lazy man's synthesis 是什麼
    - alias recadd3     "st; rec_add3; b; rec_add3; dc2; rec_add3; if -K 8; bidec; st; rec_add3; dc2; rec_add3; if -g -K 6; st; rec_add3"

