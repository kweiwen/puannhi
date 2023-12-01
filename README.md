# Puannhi - An asynchronous, time-variant feedback delay networks for reverberation

Shoutout to Professor Sebastian J. Schlecht! 

Whenever my colleagues or music-friends ask how I created such a cool effect, I tell them that I met the "Gulu of Reverb" and drew the most of my inspiration from this paper: 
> Schlecht, Sebastian J and Habets, Emanuël A P, *Time-varying feedback matrices in feedback delay networks and their application in artificial reverberation*, Journal of the Acoustical Society of America, 2015.

$\mathbf{A(n)}$: The feedback **matrix** itself. It changes over time, whic is denoted by the index $n$. 

$\mathbf{A(0)}$: This is the initial feedback **matrix**. Serving as the starting point for the time-variant modulation process. Chosen to be a unitary matrix

$\mathbf{U}$: Unitary matrix.

$\Uplambda^{\Phi(n)}$: A diagonal matrix of modulation functions. 

$\mathbf{U}^H$: Unitary matrix with conjugate transpose.

If we assume there are 4 delay lines in the implementation, $\mathbf{U}$ is a Hadamard matrix, $\mathbf{U}^H$ is the conjugate transpose of a Hadamard matrix, $\phi_1(n)$, $\phi_2(n)$, $\phi_3(n)$, and $\phi_4(n)$ are triangle modulation function. It will look like the following equation. 

$$
\begin{align}
A(n) &= A(0) \mathbf{U} K^{\Phi(n)} \mathbf{U}^H \\
     &= A(0) \left( \frac{1}{2} \begin{bmatrix} 
1 & 1 & 1 & 1 \\ 
1 & -1 & 1 & -1 \\ 
1 & 1 & -1 & -1 \\ 
1 & -1 & -1 & 1 
\end{bmatrix} \right) 
\begin{bmatrix} 
\phi_1(n) & 0 & 0 & 0 \\ 
0 & \phi_2(n) & 0 & 0 \\ 
0 & 0 & \phi_3(n) & 0 \\ 
0 & 0 & 0 & \phi_4(n) 
\end{bmatrix} 
\left( \frac{1}{2} \begin{bmatrix} 
1 & 1 & 1 & 1 \\ 
1 & -1 & 1 & -1 \\ 
1 & 1 & -1 & -1 \\ 
1 & -1 & -1 & 1 
\end{bmatrix} \right)
\end{align}
$$

When it comes to implementation, it would be much easier to implement this Time-Variant FDN in `ProcessBySample()` compared to `ProcessByBlock()`; however, I have discovered another approach to modulate the FDN, inspired by matrix modulation, which modulates the delay time of the delay lines itself. The modulation function of $f_1(n)$, $f_2(n)$, $f_3(n)$, and $f_4(n)$ are sine waves in the current implementation, and it can be substituted for any shapes of waveform. 

<p align="center">
<img src="https://github.com/kweiwen/puannhi/assets/15021145/565e187f-701d-4f9e-ac7e-062b86b5de8f.JPG" width="480">
</p>
