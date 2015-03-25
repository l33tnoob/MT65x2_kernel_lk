package com.mediatek.calendarimporter.vcalendar;

import android.util.Log;

import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.valuetype.Charset;
import com.mediatek.vcalendar.valuetype.Recur;

import junit.framework.TestCase;

public class ValuetypeTest extends TestCase {
    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_Rescur() {
        String rRule = "D1 #0";
        Recur.updateRRuleToRfc5545Version(rRule);
        VCalendar.setVCalendarVersion("VERSION:1.0");
        String dailyRule = "D1 #0";
        Recur.updateRRuleToRfc5545Version(dailyRule);
        String weeklyRule = "W1 TH #0";
        Recur.updateRRuleToRfc5545Version(weeklyRule);
        String monthlyRule = "MP1 4+ TH #0";
        Recur.updateRRuleToRfc5545Version(monthlyRule);
        String yearlyRule = "YD1 209 #0";
        Recur.updateRRuleToRfc5545Version(yearlyRule);
        String Rfc5545VersionRule = "FREQ=WEEKLY;BYDAY=TH;WKST=MO";
        Recur.updateRRuleToRfc5545Version(Rfc5545VersionRule);
        VCalendar.setVCalendarVersion("VERSION:2.0");
    }

    public void test02_Charset() {
        assertNotNull(Charset.decodeBaseQuotedPrintable("test"));
        assertNotNull(Charset.decodeQuotedPrintable("test_test", "US-ASCII"));
        assertNull(Charset.decoding(null, "US-ASCII"));
        assertNull(Charset.decoding("test", "US-ASCII"));
        assertNull(Charset.decoding("=test", "US-ASCII"));
        assertNull(Charset.decoding("=tedst", "US-ASCII"));
        assertNotNull(Charset.decoding("=ae", "US-ASCII"));
        assertNotNull(Charset.decoding("=ae=12=FF=cb=3b=12", "US-ASCII"));

        assertEquals("", Charset.encodeQuotedPrintable(null, "US-ASCII"));
        assertNotNull(Charset.encodeQuotedPrintable("testtesttesttesttesttesttesttesttesttest",
                "US-ASCII"));
        assertNull(Charset.encoding(null, "US-ASCII"));
        assertEquals("=74=65=73=74", Charset.encoding("test", "US-ASCII"));
    }
}
