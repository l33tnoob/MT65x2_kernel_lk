package com.mediatek.common.widget.tests.listview;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SectionIndexer;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@TargetApi(11)
public class ListWithFastScroller extends Activity {

    private MyContentAdapter mAdapter;
    private ListView mListView;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // adapter
        mAdapter = new MyContentAdapter(this, getData());

        // listview
        mListView = new ListView(this);
        mListView.setFastScrollEnabled(true);
        mListView.setFastScrollAlwaysVisible(true);
        mListView.setVerticalScrollBarEnabled(false);
                
        mListView.setAdapter(mAdapter);

        setContentView(mListView);
    }
    
    public ListView getListView() {
        return mListView;
    }

    private ArrayList<ListItem> getData() {

        ArrayList<ListItem> myList = new ArrayList<ListItem>();

        String c = " ";
        for (int i = 0; i < mStrings.length; i++) {
            if (!match(String.valueOf(mStrings[i].charAt(0)), c)) {
                c = String.valueOf(mStrings[i].charAt(0));
                ListItem item = new ListItem();
                item.category = ListItem.CATEGORY_HEADER;
                item.title = c;
                myList.add(item);
            }

            ListItem item = new ListItem();
            item.category = ListItem.CATEGORY_NORMAL;
            item.title = mStrings[i];            

            myList.add(item);
        }

        return myList;
    }
    
    private boolean match(String value, String keyword) {
        if (value == null || keyword == null)
            return false;
        if (keyword.length() > value.length())
            return false;

        int i = 0, j = 0;
        do {
            {
                if (keyword.charAt(j) == value.charAt(i)) {
                    i++;
                    j++;
                } else if (j > 0)
                    break;
                else
                    i++;
            }
        } while (i < value.length() && j < keyword.length());

        return (j == keyword.length()) ? true : false;
    }
    
    
    public class MyContentAdapter extends ArrayAdapter<ListItem> implements
            SectionIndexer {

        private String mSections = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        // private Context mContext;
        private LayoutInflater mInflater;
        private int[] mPositions;
        private int mCount;

        private class ViewHolder {
            
            TextView title;
        }

        public MyContentAdapter(Context context, List<ListItem> objects) {
            super(context, 0, objects);

            mInflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);

            mPositions = new int[mSections.length()];
            int index = 0;
            String c = "A";
            for (int i = 0; i < objects.size(); i++) {
                ListItem item = objects.get(i);
                if (!match(String.valueOf(item.title.charAt(0)),
                        c)) {
                    c = String.valueOf(item.title.charAt(0));
                    mPositions[++index] = i;
                }
            }
            mCount = getCount();
        }

        @Override
        public int getItemViewType(int position) {
            ListItem item = getItem(position);
            return item.category;
        }

        @Override
        public boolean areAllItemsEnabled() {
            return false; // because of header
        }

        @Override
        public boolean isEnabled(int position) {
            return getItemViewType(position) != ListItem.CATEGORY_HEADER;
        }

        @Override
        public int getViewTypeCount() {
            return 2; // CATEGORY_HEADER, CATEGORY_NORMAL
        }

        @Override
        public int getPositionForSection(int section) {

            if (section >= mSections.length()) {
                return getCount();
            } else if (section < 0) {
                return -1;
            } else {
                return mPositions[section];
            }

        }

        @Override
        public int getSectionForPosition(int position) {
            if (position < 0) {
                return -1;
            } else if (position >= mCount) {
                return mSections.length() - 1;
            }

            int index = Arrays.binarySearch(mPositions, position);

            /*
             * Consider this example: section positions are 0, 3, 5; the
             * supplied position is 4. The section corresponding to position 4
             * starts at position 3, so the expected return value is 1. Binary
             * search will not find 4 in the array and thus will return
             * -insertPosition-1, i.e. -3. To get from that number to the
             * expected value of 1 we need to negate and subtract 2.
             */
            return index >= 0 ? index : -index - 2;
        }

        @Override
        public Object[] getSections() {
            String[] sections = new String[mSections.length()];
            for (int i = 0; i < mSections.length(); i++)
                sections[i] = String.valueOf(mSections.charAt(i));
            return sections;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            final ListItem item = getItem(position);
            final int type = item.category;

            if (convertView == null) {
                holder = new ViewHolder();
                switch (type) {
                case ListItem.CATEGORY_HEADER:
                    convertView = new TextView(getContext(), null,
                            android.R.attr.listSeparatorTextViewStyle);
                    holder.title = (TextView) convertView;
                    break;

                case ListItem.CATEGORY_NORMAL:
                    // fall through
                default:
                    convertView = mInflater.inflate(android.R.layout.simple_list_item_1,
                            parent, false);
                    holder.title = (TextView) convertView
                            .findViewById(android.R.id.text1);
                    break;
                }
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            // All view fields must be updated every time, because the view may
            // be recycled
            switch (type) {
            case ListItem.CATEGORY_HEADER:
                holder.title.setText(item.title);
                break;

            case ListItem.CATEGORY_NORMAL:
                // fall through
            default:
                holder.title.setText(item.title);                
                break;
            }

            return convertView;
        }
    }

    public class ListItem {

        public static final int CATEGORY_HEADER = 0;
        public static final int CATEGORY_NORMAL = 1;

        public int category;
        public String title;        

        public ListItem() {
        }

        @Override
        public String toString() {
            if (category == CATEGORY_HEADER) {
                return "";
            } else {
                return title;
            }
        }

    }

   
    

    private String[] mStrings = { "Abbaye de Belloc",
            "Abbaye du Mont des Cats", "Abertam", "Abondance", "Ackawi",
            "Acorn", "Adelost", "Affidelice au Chablis", "Afuega'l Pitu",
            "Airag", "Airedale", "Aisy Cendre", "Allgauer Emmentaler",
            "Alverca", "Ambert", "American Cheese", "Ami du Chambertin",
            "Anejo Enchilado", "Anneau du Vic-Bilh", "Anthoriro", "Appenzell",
            "Aragon", "Ardi Gasna", "Ardrahan", "Armenian String",
            "Aromes au Gene de Marc", "Asadero", "Asiago", "Aubisque Pyrenees",
            "Autun", "Avaxtskyr", "Baby Swiss", "Babybel",
            "Baguette Laonnaise", "Bakers", "Baladi", "Balaton", "Bandal",
            "Banon", "Barry's Bay Cheddar", "Basing", "Basket Cheese",
            "Bath Cheese", "Bavarian Bergkase", "Baylough", "Beaufort",
            "Beauvoorde", "Beenleigh Blue", "Beer Cheese", "Bel Paese",
            "Bergader", "Bergere Bleue", "Berkswell", "Beyaz Peynir",
            "Bierkase", "Bishop Kennedy", "Blarney", "Bleu d'Auvergne",
            "Bleu de Gex", "Bleu de Laqueuille", "Bleu de Septmoncel",
            "Bleu Des Causses", "Blue", "Blue Castello", "Blue Rathgore",
            "Blue Vein (Australian)", "Blue Vein Cheeses", "Bocconcini",
            "Bocconcini (Australian)", "Boeren Leidenkaas", "Bonchester",
            "Bosworth", "Bougon", "Boule Du Roves", "Boulette d'Avesnes",
            "Boursault", "Boursin", "Bouyssou", "Bra", "Braudostur",
            "Breakfast Cheese", "Brebis du Lavort", "Brebis du Lochois",
            "Brebis du Puyfaucon", "Bresse Bleu", "Brick", "Brie",
            "Brie de Meaux", "Brie de Melun", "Brillat-Savarin", "Brin",
            "Brin d' Amour", "Brin d'Amour", "Brinza (Burduf Brinza)",
            "Briquette de Brebis", "Briquette du Forez", "Broccio",
            "Broccio Demi-Affine", "Brousse du Rove", "Bruder Basil",
            "Brusselae Kaas (Fromage de Bruxelles)", "Bryndza",
            "Buchette d'Anjou", "Buffalo", "Burgos", "Butte", "Butterkase",
            "Button (Innes)", "Buxton Blue", "Cabecou", "Caboc", "Cabrales",
            "Cachaille", "Caciocavallo", "Caciotta", "Caerphilly",
            "Cairnsmore", "Calenzana", "Cambazola", "Camembert de Normandie",
            "Canadian Cheddar", "Canestrato", "Cantal", "Caprice des Dieux",
            "Capricorn Goat", "Capriole Banon", "Carre de l'Est",
            "Casciotta di Urbino", "Cashel Blue", "Castellano", "Castelleno",
            "Castelmagno", "Castelo Branco", "Castigliano", "Cathelain",
            "Celtic Promise", "Cendre d'Olivet", "Cerney", "Chabichou",
            "Chabichou du Poitou", "Chabis de Gatine", "Chaource", "Charolais",
            "Chaumes", "Cheddar", "Cheddar Clothbound", "Cheshire", "Chevres",
            "Chevrotin des Aravis", "Chontaleno", "Civray",
            "Coeur de Camembert au Calvados", "Coeur de Chevre", "Colby",
            "Cold Pack", "Comte", "Coolea", "Cooleney", "Coquetdale",
            "Corleggy", "Cornish Pepper", "Cotherstone", "Cotija",
            "Cottage Cheese", "Cottage Cheese (Australian)", "Cougar Gold",
            "Coulommiers", "Coverdale", "Crayeux de Roncq", "Cream Cheese",
            "Cream Havarti", "Crema Agria", "Crema Mexicana", "Creme Fraiche",
            "Crescenza", "Croghan", "Crottin de Chavignol",
            "Crottin du Chavignol", "Crowdie", "Crowley", "Cuajada", "Curd",
            "Cure Nantais", "Curworthy", "Cwmtawe Pecorino",
            "Cypress Grove Chevre", "Danablu (Danish Blue)", "Danbo",
            "Danish Fontina", "Daralagjazsky", "Dauphin", "Delice des Fiouves",
            "Denhany Dorset Drum", "Derby", "Dessertnyj Belyj", "Devon Blue",
            "Devon Garland", "Dolcelatte", "Doolin", "Doppelrhamstufel",
            "Dorset Blue Vinney", "Double Gloucester", "Double Worcester",
            "Dreux a la Feuille", "Dry Jack", "Duddleswell", "Dunbarra",
            "Dunlop", "Dunsyre Blue", "Duroblando", "Durrus",
            "Dutch Mimolette (Commissiekaas)", "Edam", "Edelpilz",
            "Emental Grand Cru", "Emlett", "Emmental", "Epoisses de Bourgogne",
            "Esbareich", "Esrom", "Etorki", "Evansdale Farmhouse Brie",
            "Evora De L'Alentejo", "Exmoor Blue", "Explorateur", "Feta",
            "Feta (Australian)", "Figue", "Filetta", "Fin-de-Siecle",
            "Finlandia Swiss", "Finn", "Fiore Sardo", "Fleur du Maquis",
            "Flor de Guia", "Flower Marie", "Folded",
            "Folded cheese with mint", "Fondant de Brebis", "Fontainebleau",
            "Fontal", "Fontina Val d'Aosta", "Formaggio di capra", "Fougerus",
            "Four Herb Gouda", "Fourme d' Ambert", "Fourme de Haute Loire",
            "Fourme de Montbrison", "Fresh Jack", "Fresh Mozzarella",
            "Fresh Ricotta", "Fresh Truffles", "Fribourgeois", "Friesekaas",
            "Friesian", "Friesla", "Frinault", "Fromage a Raclette",
            "Fromage Corse", "Fromage de Montagne de Savoie", "Fromage Frais",
            "Fruit Cream Cheese", "Frying Cheese", "Fynbo", "Gabriel",
            "Galette du Paludier", "Galette Lyonnaise",
            "Galloway Goat's Milk Gems", "Gammelost", "Gaperon a l'Ail",
            "Garrotxa", "Gastanberra", "Geitost", "Gippsland Blue", "Gjetost",
            "Gloucester", "Golden Cross", "Gorgonzola", "Gornyaltajski",
            "Gospel Green", "Gouda", "Goutu", "Gowrie", "Grabetto", "Graddost",
            "Grafton Village Cheddar", "Grana", "Grana Padano", "Grand Vatel",
            "Grataron d' Areches", "Gratte-Paille", "Graviera", "Greuilh",
            "Greve", "Gris de Lille", "Gruyere", "Gubbeen", "Guerbigny",
            "Halloumi", "Halloumy (Australian)", "Haloumi-Style Cheese",
            "Harbourne Blue", "Havarti", "Heidi Gruyere", "Hereford Hop",
            "Herrgardsost", "Herriot Farmhouse", "Herve", "Hipi Iti",
            "Hubbardston Blue Cow", "Hushallsost", "Iberico", "Idaho Goatster",
            "Idiazabal", "Il Boschetto al Tartufo", "Ile d'Yeu",
            "Isle of Mull", "Jarlsberg", "Jermi Tortes", "Jibneh Arabieh",
            "Jindi Brie", "Jubilee Blue", "Juustoleipa", "Kadchgall", "Kaseri",
            "Kashta", "Kefalotyri", "Kenafa", "Kernhem", "Kervella Affine",
            "Kikorangi", "King Island Cape Wickham Brie", "King River Gold",
            "Klosterkaese", "Knockalara", "Kugelkase", "L'Aveyronnais",
            "L'Ecir de l'Aubrac", "La Taupiniere", "La Vache Qui Rit",
            "Laguiole", "Lairobell", "Lajta", "Lanark Blue", "Lancashire",
            "Langres", "Lappi", "Laruns", "Lavistown", "Le Brin",
            "Le Fium Orbo", "Le Lacandou", "Le Roule", "Leafield", "Lebbene",
            "Leerdammer", "Leicester", "Leyden", "Limburger",
            "Lincolnshire Poacher", "Lingot Saint Bousquet d'Orb", "Liptauer",
            "Little Rydings", "Livarot", "Llanboidy", "Llanglofan Farmhouse",
            "Loch Arthur Farmhouse", "Loddiswell Avondale", "Longhorn",
            "Lou Palou", "Lou Pevre", "Lyonnais", "Maasdam", "Macconais",
            "Mahoe Aged Gouda", "Mahon", "Malvern", "Mamirolle", "Manchego",
            "Manouri", "Manur", "Marble Cheddar", "Marbled Cheeses",
            "Maredsous", "Margotin", "Maribo", "Maroilles", "Mascares",
            "Mascarpone", "Mascarpone (Australian)", "Mascarpone Torta",
            "Matocq", "Maytag Blue", "Meira", "Menallack Farmhouse",
            "Menonita", "Meredith Blue", "Mesost", "Metton (Cancoillotte)",
            "Meyer Vintage Gouda", "Mihalic Peynir", "Milleens", "Mimolette",
            "Mine-Gabhar", "Mini Baby Bells", "Mixte", "Molbo",
            "Monastery Cheeses", "Mondseer", "Mont D'or Lyonnais", "Montasio",
            "Monterey Jack", "Monterey Jack Dry", "Morbier",
            "Morbier Cru de Montagne", "Mothais a la Feuille", "Mozzarella",
            "Mozzarella (Australian)", "Mozzarella di Bufala",
            "Mozzarella Fresh, in water", "Mozzarella Rolls", "Munster",
            "Murol", "Mycella", "Myzithra", "Naboulsi", "Nantais",
            "Neufchatel", "Neufchatel (Australian)", "Niolo", "Nokkelost",
            "Northumberland", "Oaxaca", "Olde York", "Olivet au Foin",
            "Olivet Bleu", "Olivet Cendre", "Orkney Extra Mature Cheddar",
            "Orla", "Oschtjepka", "Ossau Fermier", "Ossau-Iraty", "Oszczypek",
            "Oxford Blue", "P'tit Berrichon", "Palet de Babligny", "Paneer",
            "Panela", "Pannerone", "Pant ys Gawn", "Parmesan (Parmigiano)",
            "Parmigiano Reggiano", "Pas de l'Escalette", "Passendale",
            "Pasteurized Processed", "Pate de Fromage", "Patefine Fort",
            "Pave d'Affinois", "Pave d'Auge", "Pave de Chirac",
            "Pave du Berry", "Pecorino", "Pecorino in Walnut Leaves",
            "Pecorino Romano", "Peekskill Pyramid", "Pelardon des Cevennes",
            "Pelardon des Corbieres", "Penamellera", "Penbryn", "Pencarreg",
            "Perail de Brebis", "Petit Morin", "Petit Pardou", "Petit-Suisse",
            "Picodon de Chevre", "Picos de Europa", "Piora",
            "Pithtviers au Foin", "Plateau de Herve", "Plymouth Cheese",
            "Podhalanski", "Poivre d'Ane", "Polkolbin", "Pont l'Eveque",
            "Port Nicholson", "Port-Salut", "Postel", "Pouligny-Saint-Pierre",
            "Pourly", "Prastost", "Pressato", "Prince-Jean",
            "Processed Cheddar", "Provolone", "Provolone (Australian)",
            "Pyengana Cheddar", "Pyramide", "Quark", "Quark (Australian)",
            "Quartirolo Lombardo", "Quatre-Vents", "Quercy Petit",
            "Queso Blanco", "Queso Blanco con Frutas --Pina y Mango",
            "Queso de Murcia", "Queso del Montsec", "Queso del Tietar",
            "Queso Fresco", "Queso Fresco (Adobera)", "Queso Iberico",
            "Queso Jalapeno", "Queso Majorero", "Queso Media Luna",
            "Queso Para Frier", "Queso Quesadilla", "Rabacal", "Raclette",
            "Ragusano", "Raschera", "Reblochon", "Red Leicester",
            "Regal de la Dombes", "Reggianito", "Remedou", "Requeson",
            "Richelieu", "Ricotta", "Ricotta (Australian)", "Ricotta Salata",
            "Ridder", "Rigotte", "Rocamadour", "Rollot", "Romano",
            "Romans Part Dieu", "Roncal", "Roquefort", "Roule",
            "Rouleau De Beaulieu", "Royalp Tilsit", "Rubens", "Rustinu",
            "Saaland Pfarr", "Saanenkaese", "Saga", "Sage Derby",
            "Sainte Maure", "Saint-Marcellin", "Saint-Nectaire",
            "Saint-Paulin", "Salers", "Samso", "San Simon", "Sancerre",
            "Sap Sago", "Sardo", "Sardo Egyptian", "Sbrinz", "Scamorza",
            "Schabzieger", "Schloss", "Selles sur Cher", "Selva", "Serat",
            "Seriously Strong Cheddar", "Serra da Estrela", "Sharpam",
            "Shelburne Cheddar", "Shropshire Blue", "Siraz", "Sirene",
            "Smoked Gouda", "Somerset Brie", "Sonoma Jack",
            "Sottocenare al Tartufo", "Soumaintrain", "Sourire Lozerien",
            "Spenwood", "Sraffordshire Organic", "St. Agur Blue Cheese",
            "Stilton", "Stinking Bishop", "String", "Sussex Slipcote",
            "Sveciaost", "Swaledale", "Sweet Style Swiss", "Swiss",
            "Syrian (Armenian String)", "Tala", "Taleggio", "Tamie",
            "Tasmania Highland Chevre Log", "Taupiniere", "Teifi", "Telemea",
            "Testouri", "Tete de Moine", "Tetilla", "Texas Goat Cheese",
            "Tibet", "Tillamook Cheddar", "Tilsit", "Timboon Brie", "Toma",
            "Tomme Brulee", "Tomme d'Abondance", "Tomme de Chevre",
            "Tomme de Romans", "Tomme de Savoie", "Tomme des Chouans",
            "Tommes", "Torta del Casar", "Toscanello", "Touree de L'Aubier",
            "Tourmalet", "Trappe (Veritable)", "Trois Cornes De Vendee",
            "Tronchon", "Trou du Cru", "Truffe", "Tupi", "Turunmaa",
            "Tymsboro", "Tyn Grug", "Tyning", "Ubriaco", "Ulloa",
            "Vacherin-Fribourgeois", "Valencay", "Vasterbottenost", "Venaco",
            "Vendomois", "Vieux Corse", "Vignotte", "Vulscombe",
            "Waimata Farmhouse Blue", "Washed Rind Cheese (Australian)",
            "Waterloo", "Weichkaese", "Wellington", "Wensleydale",
            "White Stilton", "Whitestone Farmhouse", "Wigmore",
            "Woodside Cabecou", "Xanadu", "Xynotyro", "Yarg Cornish",
            "Yarra Valley Pyramid", "Yorkshire Blue", "Zamorano",
            "Zanetti Grana Padano", "Zanetti Parmigiano Reggiano" };
}
