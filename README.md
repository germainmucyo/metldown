# Lab 6: Meltdown Attack

## Overview
This lab explores **side-channel** and **covert-channel attacks**, focusing on exploiting microarchitectural weaknesses in modern CPUs. The main modules of this lab include:

1. **Covert Channel Reliability** (Flush+Reload method)  
2. **Exception Suppression**  
3. **Meltdown Attack**  

---

## Modules

### 1. Covert Channel Reliability

This module uses the **Flush+Reload (F+R)** technique to establish a covert channel through shared CPU cache states.

- **Method**: Data transmission over 100,000 iterations using random bytes.  
- **Results**: Average accuracy ~**92%**.  
- **Key Factors**: Accuracy drops caused by **CPU noise** and improper threshold calibration.

| **Run**  | **Threshold** | **Accuracy**  |
|----------|---------------|---------------|
| Run 1    | 150           | 90.19%        |
| Run 2    | 148           | 95.23%        |
| Run 3    | 159           | 93.01%        |
| Run 4    | 157           | 15.64%        |
| Run 5    | 148           | 86.85%        |

---

### 2. Exception Suppression

To prevent crashes during speculative execution, **exception suppression** was implemented. This module laid the groundwork for speculative attacks (e.g., Meltdown).

- **Objective**: Suppress segmentation faults during illegal memory access attempts.  
- **Outcome**: Successfully managed exceptions, enabling safe continuation of attacks.  

---

### 3. Meltdown Attack

The **Meltdown attack** exploits **speculative execution** to leak secrets from **kernel memory**.

- **Techniques Used**:  
   - Side channel: **Flush+Reload**  
   - Speculative execution to bypass memory isolation.  

- **Observations**:  
   - Successful kernel memory leak.  
   - Mitigations (e.g., **KPTI**) impacted results on patched systems.

**CPU Model**: `Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz`  

---

## Challenges

1. **Segmentation Faults**:  
   - Successfully suppressed frequent segmentation faults during speculative execution.  

2. **System Mitigations**:  
   - Kernel Page Table Isolation (**KPTI**) and other CPU-level patches limited the effectiveness of Meltdown on some systems.

---

## Conclusion

This lab successfully demonstrated the **Meltdown attack** and **side-channel covert communication techniques**. It highlights the dangers of **speculative execution vulnerabilities** and the importance of mitigations in modern systems.


### Author: **Germain Mucyo**  
**Email**: [mucyo.g@northeastern.edu](mailto:mucyo.g@northeastern.edu)  
**Course**: **EECE5699 - Computer Hardware and Systems Security**  
**Date**: **December 6, 2024**  

---

## References

- **Flush+Reload Technique**  
- **Meltdown Attack**: [https://meltdownattack.com/](https://meltdownattack.com/)  

---

### License  
[MIT License](LICENSE)  
