以c++模擬組合語言3-bit Histroy Predictor。

首先輸入一個正整數 = 2 ^ n, n >= 0，表示entry的數量，再輸入要模擬的程式碼(MIPS)，格式如test input.txt。

進入execute()，先遍歷一次整個Input的program，訂好每行程式使用的entry和Label指向的PC。

接著開始decode，用stringstream和string的substr()把instruction的operand和operator抓好，預測是否會發生branch後執行。

可以處理的instruction有add, sub, mul, div, rem, and, or, xor, beq, bne, bge, blt, li, addi, andi, ori, xori，可以做正確的運算和branch。

執行完畢後檢視預測和實際的結果是否一樣，接著將目前所有entry的內容輸出
(包含正在執行哪一行程式、此行程式使用的entry，預測結果和實際結果、每個entry目前的狀態、history、每格是SN,WN,WT,ST和預測失敗的次數)

輸出完畢後依照執行結果更動entry的內容。


