# VH-20: Vintage Hue — Add vintage character to sounds

VH-20 is a multi-effect plugin capable of imparting tube distortion, soft noise, and tape wobble to sounds. Inspired by XLN Audio’s popular RC-20 Retro Color product, it is intended for producers looking to add a dash of the warmth and character missing in the digital era.

![](https://lh5.googleusercontent.com/JBnMO1ogGMjZPXKDC5crzFRtvziSxrluL4YuN6dRHrqXv2Q2mYF6YKhP63HNnmWqvSAQgbUZ2lH-5hccxmTgjj87LRzPe7fcU_s_FYrIWHXIBK0ceIMpm3VggKRL1Ofuj-a4sqK-ct1jZ-gH5NRUdjo)

## Interface

RC-20 provides six effect modules — noise, wobble, distortion, bitcrushing, reverb, and magnetic tape artifacts — in addition to general gain, EQ, and mix controls. 

In VH-20, I was able to support distortion (“Grit”), noise (“Film”), and wobble (“Wear”). The effects are applied in the order presented by the user interface from left to right, designed to first redefine the frequency spectrum of the input signal, then soften the distortion with a thin layer of noise, and finally wobbling the cumulative effect.

The _Grit_ module produces even and odd harmonics of the input signal. As the Grit Amount is dialed up, the signal is driven with increasing amplitude into an increasingly non-linear waveshape.

Next, the _Film_ module adds white or pink noise to the distorted signal. Dialing up the Film Amount knob increases the gain, ranging from a thin film of noise atop the signal to an airy blast potentially drowning it out. The Film Type selector offers white noise (“Dust”) or pink noise (“Mist”) as options, the latter of which has an attenuated high-end. 

Finally, the _Wear_ module applies a pitch wobbling effect reminiscent of aged tape. While the oscillation speed is fixed at a slow rate, dialing up the Wear Amount knob increases the magnitude of detuning that occurs.

## Signal Processing

![](https://i.imgur.com/y4heRXb.png)

The stereo plugin is represented with one signal channel for simplicity. Superscripts clarify filter order.

### Oversampling

Oversampling to twice the original sampling rate (usually, 88.2 kHz) is implemented via the classic “zero stuffing” technique. This approach is straightforward, but introduces images, which are removed with a biquad LPF[^1]. The filter also interpolates between the spaced-out values. 

### Distortion

The signal then passes through the distortion module, which uses a waveshaper algorithm to add harmonics. As the Grit Amount increases, the input is pre-amplified by up to 6.0 linear gain, or 7.8 dbFS. 

At the same time, the non-linearity of the waveshape increases by interpolating between a linear unit (y=x) and non-linear waveshape, with greater weight assigned to the latter. Together, these effects scale up the frequencies added by distortion and the degree to which the signal is rammed into the module.

![](https://lh3.googleusercontent.com/kvO5CvhcyhA2daNLEpHv9IKwylIEBuusPuBjiQNP73wS33HmMIAON_3g9RHo71WTtKv9-69rD59JCx2Z43A3Ou5pILuMSXQAb-8DaGAE8tzGf0qxi9NkpS7ej-WBF5OcSqqGNX7nFHXiQlU0qQup4ig)

Finally, some gain reduction (up to -6 dbFS) is applied to partially compensate for the pre-amp, taming the extent of loudness increase.

### Noise

The distorted signal then passes to the noise module. Noise generation happens with a straightforward random float generator, which samples values from a uniform random distribution and applies gain based on the Film Amount. However, when Mist is selected from Film Type, a -3db/octave pinking filter[^2] is applied to the noise signal before adding it in.

![](https://lh6.googleusercontent.com/OJ3vqNP93Ma5trIay_wCnQMy7zy8jzyf1eIUmgfqXOdiyTFgHrQqo-bBLhOTjGhc7VKVgmrD9QLHvB1VPW0NVuAnpqLAULscJXa8WU1Gg5a2xhnfArHqNJ8il7H-eu0qbKx6gVwFJpHAETBECNiw3v0)

Originally, only distortion was performed on the oversampled signal; noise and wobble were added after downsampling back to the original sampling rate. However, the noise sounds less harsh when generated at the oversampled rate, fulfilling its intent as a subtle background texture.

### Wobble

The last stage is tape wobble, which borrows RC-20’s default LFO speed of 0.4 Hz. At a Wear Amount of 0, the module is disabled by setting the delay interval to [0, 0] ms.
As Wear Amount increases, the delay interval scales from 0-3 ms to 20-23 ms. 

![](https://lh3.googleusercontent.com/TPDclSERlxKgXxfBJTxffaf5MnYy1jVzWZuAc1ps4LGBmeL87Hd86kTl0QfmVnhxEwlcADEeX_TBbdlW8rHKXSOT8xC8Q6nReHmKepvUH5UFq-eiFq3JyGgqJ4npVNMqPSkasuaiB2npnhH1_YnhNE4)

The slow rate and small, 3 ms depth of the LFO prevent the effect from slipping into the realm of phasing or chorusing[^3]. Yet the adjustable minimum delay time, up to 20 ms, allows for hefty pitch-bending to be applied.

As with noise, it was found that performing wobble to the oversampled signal minimized unpleasant artifacts.

### Downsampling

Downsampling is performed by decimating the sample rate to just every other sample, starting with the 0th sample. Before doing so, however, the signal is anti-aliased with a 66th-order FIR filter[^4].

## Application

### Development & Testing

The most frequent and direct comparison with RC-20 during the development and testing of VH-20 was for the distortion function. Extensive work went into modeling RC-20’s Tube Pair distortion type.

By generating a 100 Hz (G2 +35 cents) sine wave and dialing up Tube Pair to 85%, I generated the following frequency response and tabulated harmonics and respective amplitudes.

![](https://lh6.googleusercontent.com/EXtM0Peif5vnbDQ0zemck-DygYaJliBpCZRmDszWL8U5jFF-Jxb4q-B3ngzGqd-CKrZCtgi0M2CYJ7j8c4rEE2vou2LgOkHuTVFaNGP6myrpBOJ_vRfAcYCunMSNSYJOKNHoBhU6TjFTFop1LKpUbv8)

Compare the original sine tone vs. the RC-20 distorted version in “sine_pure_vs_RC20.wav.”

The first and most immediate approach for modeling this frequency response, abundant with even and odd harmonics, was through harmonic synthesis, specifically summing [Chebyshev polynomials](http://sites.music.columbia.edu/cmc/MusicAndComputers/chapter4/04_06.php) as a means of recreating RC-20’s waveshape. While [spectral matching](http://musicweb.ucsd.edu/~trsmyth/waveshaping/Example_Spectral_Matching.html) is [theoretically](https://dsp.stackexchange.com/questions/5959/add-odd-even-harmonics-to-signal) [possible](http://www.rs-met.com/documents/tutorials/Waveshaping.pdf), I stumbled into massive DC offsets generating even-order harmonics (as their Chebyshev waveshapes don’t cross the origin).


The 2nd-order Chebyshev-processed wave, in purple, has twice the frequency of the original sine wave, in red. Yet it does not cross the origin, hence producing DC offset.

![](https://lh6.googleusercontent.com/LIjP2KrIZ29eZ_b8CAODlJVuB_LqjeeNa9oAQ-VG4q8aX9YogaUk_NJrLcB-gEDkYZnNKAf30cfKyvk-okYlzUxvV33sxCBo-We_9oqSYknEszCVAynxtzHqf1QJY0wvft0EHmFthpUCKF8cqyGXMD0)

By leveraging trigonometry rules, I was able to create my own transfer function, k(s), that technically produces a sine wave with double the input frequency for the [-1, 1] domain

![](https://lh3.googleusercontent.com/_F99cBNqBFFe7-g0MXId1saq2MSmX9_UjtW9qNtlNxEmiUp8FKyhqAX2etpZcdBe7vojguj9CApsgxPpVN2cY_FSpc0EIBDrPwUHpc2sJzpCmTy3ptGBcAbc72NUUYnFtRl5fpB3vlUgX3ja9F81l4g)

But the results were strange for non-sinusoidal waveforms and I also realized there were too many harmonics to produce with this line of thinking. 

My next step was to look at the RC-20-produced waveform of the output signal. A 100 Hz sine wave was converted to the following waveform by the Tube Pair setting.

![](https://lh6.googleusercontent.com/kdviWAQJ-RCw3taX0HE3bE-5_1pFYuMdPrOSQ2I48EGukA6zQqMxY-plKL9wP3Hqq7jk_jfyih79wcX92B1ZMfr4YBLGtCX8MjEiNv2YQb0mmnZtfk2AAMsx2x0_u1PUf_5HeToZUCmbdMIshKblx9E)


I reasoned that the signal must have been convolved with a function, rather than waveshaped, due to the asymmetric waveform. However, it is more likely ascribable to hysteresis, specifically, separate waveshapers for rising vs. falling signals, as pointed out by the professor in class.

My ultimate approach was to tinker with FL Studio’s waveshaper plugin and compare my own distorted sound to that of RC-20’s — playing it by ear, literally. I arrived at the following waveshape function and resulting waveform.

![](https://lh6.googleusercontent.com/eUAV1FR-pc59t9n3mZwiDZpgQbbJTFQChM3gDeAlweQmPwIl_WXspJe2bM5tpGJdFWknpgTBt8vGro7k8ih229aFUfAIKSVI6qs9Ezk3cBk74ZkoQPZAC4PQJSuMMuSQ3YjfWHwqiftTwTJrLgS59oE)

To make it easier to program within VH-20, I then made a straight-edged version.

![](https://lh5.googleusercontent.com/49kSHlrCmcmznjdCyOon7yydcm3sTGhK7p-D1_IxKBM0BOutij1NJEnMgb_QsazzUBZWsVdR-8RKtiWC5mQ0yjvXDtXCZbJFlZ-LWguzJSZO_lQP-2hawAphh_OHXFrMoHWnVJblggwDFbgAmhuQXdE)


Compare RC-20 vs. my FL waveshaper vs.  my simpler FL waveshaper in “sine_RC20_vs_waveshaper_vs_waveshaperSimple.wav”.

Then in VH-20, I implemented the simpler waveshape and compensated for some of the lost grit by implementing pre-amp and progressively stronger sloping. 

Somehow, it sounds different from my FL waveshapers — but closer to RC-20. Compare RC-20 vs. VH-20 in “sine_VH20.wav”.

 At each stage of this process, from RC-20 → first waveshaper → simpler waveshaper → VH-20 I aimed to capture the character of the previous stage, and was only somewhat successful. But in the end, I got a better result than anticipated. Though distinct from RC-20’s Tube Pair, my own Grit module is a decent tube distortion implementation in its own right.

![](https://lh6.googleusercontent.com/YtbUSoZ1Fuu1Pcq5NOy2QLTiZkpd0SetcxriSBJVY3vPEQe_w909ZpfZj1fQppH3lluE57A8MmnT844rZkrg0DZWbkfAFaiukn67qOSUmcjxH4CJ7LdhWutKqkdtYtK6Gfb00Jc0kyP0WTARfMSr4UQ)

### Production

“moodyPianoRapBeat_vh20_demo.wav” showcases an in-progress rap instrumental inspired by Machine Gun Kelly - Rap Devil and Eminem - Killshot.

After 4 bars, VH-20 with the following parameters are applied to the piano only, resulting in a more present, grainy, and eerie sound.

![](https://lh5.googleusercontent.com/MSMFJXtAKxMFm2V8S_YUtOcC15b-2NiCQJ-l0qK6oGiRY_qS1i68faFK-sqK4eFhXxy5EkhRV4578Vm-P19dU-Jnedp3-zppKZVsD95AyozgkNbGX-Ix2gTvJCO64SQ4Lsxb_mXFpDiPZHIpuwsgVC0)

The melody thereby sits better in the mix with stronger color and character and less of an “in-the-box” sound library feel.

## Addendum

In an attempt to model RC-20’s Iron Transformer distortion type, I encountered interesting [discussions](https://www.kvraudio.com/forum/viewtopic.php?t=461007) and [literature](https://www.mennovanderveen.nl/images/onderzoek-ontwikkeling/publicaties/Partridge_1939_Distortion_in_Xfrmr_Cores.pdf) on transformers.

[^1]: https://www.earlevel.com/main/2021/09/02/biquad-calculator-v3/
[^2]: https://www.musicdsp.org/en/latest/Filters/85-1st-and-2nd-order-pink-noise-filters.html
[^3]: https://www.marshallforum.com/threads/what-is-a-detune-effect-and-when-would-you-use-it.88037/#post-1468945
[^4]: https://fiiir.com/




