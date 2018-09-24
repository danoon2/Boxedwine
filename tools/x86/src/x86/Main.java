package x86;

import java.io.FileOutputStream;
import java.util.Vector;

public class Main {
    static public void main(String[] args) {
        Vector<Base> ops = new Vector<Base>();
        ops.add(new Arith());
        ops.add(new IncDec());
        ops.add(new PushPop());
        ops.add(new Strings());
        ops.add(new Shift());
        ops.add(new Conditions());
        ops.add(new SetCC());
        ops.add(new Xchg());
        ops.add(new Bit());
        ops.add(new Other());
        ops.add(new Jump());
        ops.add(new Move());

        try {
            FileOutputStream fos_init = new FileOutputStream("../common/cpu_init.h");

            for (Base b : ops) {
                b.generate(fos_init);
            }
            fos_init.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
