package inria.mjolnir.fancyapp;

/**
 * Created by Sebastien Poulmane on 10/10/2017.
 */

public enum TimeTag {
    PIEZZO(0),
    TOUCH(1),
    DISPLAY(2),
    LED(3);

    public static int length = TimeTag.values().length;

    private int _value;

    TimeTag(int value) {
        this._value = value;
    }

    public int getValue(){
        return this._value;
    }
}
