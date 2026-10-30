# Rotation regression fixture

Run `python3 tests/test_rotation.py --operations-root /path/to/wow-operations`
on the guarded WSL build host. The test compiles the selected rotation method
bodies with interface doubles and checks learned/active spell handling and
three faulty variants. Temporary outputs are created under the operations
repository's `work/` directory and removed after the run. This does not prove
a complete server build or gameplay behavior.
