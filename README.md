# Voices Audacity v0.3.0 — Liberty Music Software
Development prototype, not for commercial distribution. JUCE 8.0.10 / C++17.

Build on macOS: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 && cmake --build build --config Release --parallel 2`.
Windows: `cmake -S . -B build -A x64` then `cmake --build build --config Release --parallel 2`.
Portable tests without JUCE: `cmake -S . -B dsp -DVOICES_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release && cmake --build dsp && ctest --test-dir dsp --output-on-failure`.

The UI is drawn in code, not a screenshot. The 100 factory presets are deterministic parameter combinations, not yet auditioned. User presets use timestamp-based names; MIDI CC learn maps channel+CC for rotary controls and persists with project state. Global learn: enable, move a knob, move a MIDI CC. Right-click a knob to learn/clear. Browser search, category and project-local favorites. The host must route MIDI to the effect. French UI defaults to FR. Native Liberty DAW adapter not yet implemented.

DSP limitations: monophonic detector with ~43–47 ms analysis window at common sample rates, ~5–6 ms hop, DC rejection and stereo cancellation fallback; dual delay pitch shifting, not a production-quality formant-preserving algorithm. The new experimental spectral formant engine uses weighted overlap-add FFT, smoothed log spectral envelopes, independent frequency-axis remapping and bounded gain. It is not a validated vocal-tract model. Added fixed processing delay is 1024 samples (2048 above 64 kHz), plus the existing nominal ~10 ms dry delay; actual pitch shifter delay remains variable. No verified live latency claim. Reverb is a simple feedback delay, effects bypass not crossfaded, pitch transitions may click. No real-vocal listening tests, CPU profiling or plugin-host validation. v0.1.0 and v0.2.0 CI passed on macOS ARM64 and Windows x64; v0.3.0 CI and listening validation are required. No signed installers. CI is configured to upload built bundles after tests.

## v0.2.0 Vocal Precision
Shorter detection window; note hysteresis in Natural mode; transition timing on note changes; continuous tolerance deadband; empty custom scales disable tuning; correction meter clears when bypassed. Synthetic tests cover harmonic onset, DC rejection, opposite-phase stereo, correction direction and custom mask. Actual vocal quality and live latency remain unverified; formant control remains spectral colour only.

## v0.3.0 Vocal Formant Engine
Experimental spectral-envelope compensation/remapping (±12 semitones) while keeping the underlying harmonic frequencies. Active on voiced material with tuning enabled; unity path has fixed WOLA delay. Existing parameter ID retained for automation compatibility. Dry/wet branches aligned for the new fixed delay, host reports nominal latency. New tests cover unity impulse reconstruction, finite output and synthetic harmonic-envelope redistribution at three rates. The global power bypass remains immediate for compatibility; real-vocal quality, exact pitch-shifter timing, CPU and host behaviour remain unverified. Live mode, effect crossfades and independent DSP quality review are not completed.
