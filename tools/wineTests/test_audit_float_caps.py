"""Capability comparison must reject missing support and incomplete evidence."""
import copy
import unittest

from auditFloatCaps import compare


def fixture():
    pairs = [('R16F','R16F'),('G16R16F','RG16F'),('A16B16G16R16F','RGBA16F'),
             ('R32F','R32F'),('G32R32F','RG32F'),('A32B32G32R32F','RGBA32F')]
    raw = dict(complete=True, passed=True, diagnostics=[], inputs_unchanged=True,
               result=dict(passed=True, errors=[], observationSeconds=15,
                           checks=[dict(passed=True)], formats=[]))
    lines = []
    for d3d, gl in pairs:
        raw['result']['formats'].append(dict(name=gl, filtering=True, rendering=True, blending=True,
            operations=[dict(operation=n) for n in ('point','linear-mag','linear-min','render','blend')]))
        lines.append(f'FLOAT_CAP format={d3d} texture=00000000 filter=00000000 rt_texture=00000000 rt_surface=00000000 blend_texture=00000000 blend_surface=00000000')
    lines.append('FLOAT_COVERAGE attempted=42 completed=42 unsupported=0')
    return raw, '\n'.join(lines)


class FloatCapsTests(unittest.TestCase):
    def test_all_operations(self):
        result = compare(*fixture())
        self.assertTrue(result['passed'])
        self.assertEqual(result['expected_cases'], 42)

    def test_repeated_identical_capture(self):
        r, text = fixture()
        self.assertTrue(compare(r, text + '\n' + text)['passed'])

    def test_missing_optional_capability(self):
        r, text = fixture()
        self.assertFalse(compare(r, text.replace('blend_texture=00000000', 'blend_texture=8876086a', 1))['passed'])

    def test_falsely_advertised_capability(self):
        r, text = fixture()
        for f in r['result']['formats'][3:]:
            f['filtering'] = False
            f['operations'] = [op for op in f['operations'] if not op['operation'].startswith('linear-')]
        self.assertFalse(compare(r, text)['passed'])

    def test_missing_rendered_case(self):
        r, text = fixture()
        self.assertFalse(compare(r, text.replace('completed=42', 'completed=41'))['passed'])

    def test_incomplete_raw_observation(self):
        r, text = fixture()
        for key, value in [('complete',False),('passed',False),('inputs_unchanged',False),('diagnostics',['error'])]:
            broken = copy.deepcopy(r)
            broken[key] = value
            with self.assertRaises(ValueError):
                compare(broken, text)
        r['result']['observationSeconds'] = 0
        with self.assertRaises(ValueError):
            compare(r, text)

    def test_missing_raw_pixel_operation(self):
        r, text = fixture()
        r['result']['formats'][0]['operations'].pop()
        with self.assertRaises(ValueError):
            compare(r, text)

    def test_missing_or_conflicting_d3d_rows(self):
        r, text = fixture()
        with self.assertRaises(ValueError):
            compare(r, '\n'.join(text.splitlines()[1:]))
        with self.assertRaises(ValueError):
            compare(r, text + '\n' + text.splitlines()[0].replace('filter=00000000', 'filter=8876086a'))


if __name__ == '__main__':
    unittest.main()
