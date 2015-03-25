package com.mediatek.common.geocoding;

public interface IGeoCodingQuery {
    // Exposed methods to clients
    public static final String GET_INSTANCE = "getInstance";
    /**
     * Query the geographical description from the phone number.
     * @param number the phone number.
     * @return geographical description.
     * @internal
     */
    public String queryByNumber(String number);
}
