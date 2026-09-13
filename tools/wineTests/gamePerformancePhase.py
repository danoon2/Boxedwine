"""Optional stationary-scene measurements using the capture driver's observer."""


def measure_phase(action, measurements, name):
    # Warm up outside the recorded interval. The capture driver performs the
    # measurement without input, screenshots or state polling during it.
    action('wait', milliseconds=5000)
    action('wait', milliseconds=5000)
    measurements[name] = action('measure-performance', name=name, milliseconds=30000)


def measurements_pass(measurements, names):
    return (set(measurements) == set(names)
            and all(isinstance(measurements[name], dict)
                    and measurements[name].get('measurementValid') is True for name in names))
