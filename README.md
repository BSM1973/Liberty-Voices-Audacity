# Voices Audacity v0.1.0 — Liberty Music Software
Development prototype, not for commercial distribution. JUCE 8.0.10 / C++17.

Build on macOS: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 && cmake --build build --config Release --parallel 2`.
Windows: `cmake -S . -B build -A x64` then `cmake --build build --config Release --parallel 2`.
Portable tests without JUCE: `cmake -S . -B dsp -DVOICES_DSP_ONLY=ON -DCMAKE_BUILD_TYPE=Release && cmake --build dsp && ctest --test-dir dsp --output-on-failure`.

The UI is drawn in code, not a screenshot. The 100 factory presets are deterministic parameter combinations, not yet auditioned. User presets use timestamp-based names; MIDI CC learn maps channel+CC for rotary controls and persists with project state. Global learn: enable, move a knob, move a MIDI CC. Right-click a knob to learn/clear. Browser search, category and project-local favorites. The host must route MIDI to the effect. French UI defaults to FR. Native Liberty DAW adapter not yet implemented.

DSP limitations: prototype monophonic detector with ~85 ms analysis window; dual delay pitch shifting, not a production-quality formant-preserving algorithm. The formant colour control is spectral EQ only. Nominal reported dry latency ~10 ms, actual shifted signal variable; no verified live latency claim. Reverb is a simple feedback delay, effects bypass not crossfaded, pitch transitions may click. No real-vocal listening tests, CPU profiling or plugin-host validation. JUCE itself and the macOS/Windows targets were not compiled in the source-generation environment. No signed installers. CI is configured to upload built bundles after tests.
