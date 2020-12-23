import mysql.connector
import matplotlib
import matplotlib.pyplot as plt
from scipy.ndimage.filters import gaussian_filter1d
from scipy.signal import savgol_filter
MAX_ENTRIES = 20000
SIGMA = 56
WINDOW_SIZE = 7
IF_SMOOTH = True
config = {
  'user': 'dlm_user',
  'password': '-Sm4Dgt:q#?9t(qv',
  'host': '45.63.35.198',
  'database': 'dlm_db',
  'raise_on_warnings': True
}
def smooth_data(data):
	if(IF_SMOOTH):
		return gaussian_filter1d(data ,SIGMA)
	else:
		return data
ti=[]
avga=[]
maxa=[]
mina=[]
cnx = mysql.connector.connect(**config)
cursor = cnx.cursor()

query = ("SELECT time_index, avg_amp, max_amp,min_amp FROM NAS order by time_index desc limit 0,"+str(MAX_ENTRIES))
cursor.execute(query)

for (time_index, avg_amp, max_amp,min_amp) in cursor:
	ti.append(time_index)
	avga.append(avg_amp)
	maxa.append(max_amp)
	mina.append(min_amp)

dates = matplotlib.dates.date2num(ti)
plt.plot_date(dates, smooth_data(avga),linestyle='solid',marker='None')
plt.plot_date(dates, smooth_data(maxa),linestyle='solid',marker='None')
plt.plot_date(dates, smooth_data(mina),linestyle='solid',marker='None')
plt.legend(('Average', 'Max', 'Min'),loc='upper right')
plt.title('RSD record last '+str(MAX_ENTRIES)+' entries.')
plt.xlabel("Time index (UTC)")
plt.show()
cursor.close()
cnx.close()