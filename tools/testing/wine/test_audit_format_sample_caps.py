"""Negative controls for the per-format capability comparison."""
import unittest

from auditFormatSampleCaps import compare


def fixture():
    # Vary the lists deliberately; a single global maximum cannot satisfy them.
    lists = {name: [4, 2] for name in (
        "RGBA8", "DEPTH16", "DEPTH24", "DEPTH24_STENCIL8", "DEPTH32F", "DEPTH32F_STENCIL8")}
    lists.update(RGB565=[2], RGBA4=[4], RGB10_A2=[4, 2, 1],
                 R16F=[8, 2], RG16F=[2], RGBA16F=[2], R32F=[], RG32F=[4], RGBA32F=[])
    query = []
    for name, samples in lists.items():
        query.append(f"FORMAT_SAMPLES format={name} count={len(samples)} error=0")
        query.extend(f"FORMAT_SAMPLES format={name} index={i} samples={v}" for i, v in enumerate(samples))
    aliases = [("A8R8G8B8", "RGBA8"), ("X8R8G8B8", "RGBA8"), ("R5G6B5", "RGB565"),
               ("X1R5G5B5", "RGBA8"), ("A1R5G5B5", "RGBA8"), ("A4R4G4B4", "RGBA4"),
               ("A2R10G10B10", "RGB10_A2"), ("R16F", "R16F"), ("G16R16F", "RG16F"),
               ("A16B16G16R16F", "RGBA16F"), ("R32F", "R32F"), ("G32R32F", "RG32F")]
    color = ["FORMAT_CAP format=A32B32G32R32F render_target=8876086a"]
    for name, fmt in aliases:
        values = lists[fmt]
        color.append(f"FORMAT_CAP format={name} render_target=00000000")
        color.append(f"FORMAT_CAP format={name} samples=0 result=00000000 quality=1")
        color.append(f"FORMAT_CAP format={name} samples=1 result=" +
                     (f"00000000 quality={len(values)}" if values else "8876086a quality=1"))
        color.extend(f"FORMAT_CAP format={name} samples={s} result=" +
                     ("00000000" if s in values else "8876086a") + " quality=1" for s in range(2, 17))
    return "\n".join(query), "\n".join(color)


class SampleCapsTests(unittest.TestCase):
    def test_varying_lists_and_empty_float_list(self):
        result = compare(*fixture())
        self.assertTrue(result["passed"])
        self.assertEqual(len(result["comparisons"]), 13)

    def test_identical_repeated_capture(self):
        q, c = fixture()
        self.assertTrue(compare(q + "\n" + q, c + "\n" + c)["passed"])

    def test_global_maximum_inference_rejected(self):
        q, c = fixture()
        c = c.replace("format=R5G6B5 samples=4 result=8876086a", "format=R5G6B5 samples=4 result=00000000")
        self.assertFalse(compare(q, c)["passed"])

    def test_pre_remap_alias_rejected(self):
        q, c = fixture()
        c = c.replace("format=X1R5G5B5 samples=1 result=00000000 quality=2", "format=X1R5G5B5 samples=1 result=8876086a quality=1")
        self.assertFalse(compare(q, c)["passed"])

    def test_nonmaskable_quality_count_rejected(self):
        q, c = fixture()
        c = c.replace("format=A2R10G10B10 samples=1 result=00000000 quality=3", "format=A2R10G10B10 samples=1 result=00000000 quality=2")
        self.assertFalse(compare(q, c)["passed"])

    def test_missing_format_and_missing_sample_rejected(self):
        q, c = fixture()
        for broken in (q.replace("format=RGBA8", "format=MISSING"), q.replace("index=1 samples=2", "index=2 samples=2", 1)):
            with self.assertRaises(ValueError):
                compare(broken, c)
        with self.assertRaises(ValueError):
            compare(q, c.replace("format=A8R8G8B8 samples=16", "format=A8R8G8B8 samples=17"))

    def test_disagreeing_duplicate_rejected(self):
        q, c = fixture()
        with self.assertRaises(ValueError):
            compare(q + "\nFORMAT_SAMPLES format=RGBA8 index=0 samples=8", c)
        with self.assertRaises(ValueError):
            compare(q, c + "\nFORMAT_CAP format=A8R8G8B8 samples=1 result=00000000 quality=9")

    def test_query_error_rejected(self):
        q, c = fixture()
        with self.assertRaises(ValueError):
            compare(q.replace("error=0", "error=1280", 1), c)


if __name__ == "__main__":
    unittest.main()
