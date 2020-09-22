import discord
import mysql.connector
import valve.source.a2s
import asyncio
import math
import json
import random
import steam as SID
from datetime import datetime
from discord.ext import tasks, commands
from steamid import SteamID 

with open("config.json") as cfg:
    config = json.load(cfg)
    
TOKEN        = config["bot_token"]
PREFIX       = config["command_prefix"]
ICON         = config["embed_icon"]
IP           = config["server_ip"]
PORT         = config["server_port"]
SERVER       = config["server_name"]
DB_IP        = config["db_ip"]
DB_DB        = config["db_database"]
DB_USER      = config["db_user"]
DB_PASS      = config["db_pass"]
TABLE_PREFIX = config["table_prefix"]
STATS_PAGE   = config["webstats_url"]

bot = commands.Bot(command_prefix=PREFIX)
SERVER_ADDRESS = (IP, PORT)

db = {
  'user': DB_USER,
  'password': DB_PASS,
  'host': DB_IP,
  'database': DB_DB
}
    
@bot.event
async def on_ready():
    status_task.start()
 
@tasks.loop(seconds=120.0)
async def status_task():
    with valve.source.a2s.ServerQuerier(SERVER_ADDRESS) as server:
        info = server.info()
        await bot.change_presence(activity=discord.Game(name="{map}".format(**info)))
    
@bot.command(aliases=['record', 'mtop', 'maptop', 'maprecord'])
async def wr(ctx, arg):      
    await printRecord(ctx, arg, 0)
    
@bot.command(aliases=['brecord', 'btop', 'bonustop', 'bonusrecord'])
async def bwr(ctx, arg):
    await printRecord(ctx, arg, 1)
    
@bot.command()
@commands.cooldown(1, 15, commands.BucketType.user)
async def records(ctx, arg):
    steamid3 = formatSteamID3(arg)

    sql = "SELECT time, map FROM(SELECT a.map, a.time, COUNT(b.map) + 1 AS rank FROM playertimes a LEFT JOIN playertimes b ON a.time > b.time AND a.map = b.map AND a.style = b.style AND a.track = b.track WHERE a.auth = " + steamid3 + " AND a.style = 0 AND a.track = 0 GROUP BY a.map, a.time, a.jumps, a.id, a.points  ORDER BY a.map ASC) AS t WHERE rank = 1 ORDER BY map ASC"
    conn = mysql.connector.connect(**db)
    cursor = conn.cursor()
    cursor.execute(sql)
    results = cursor.fetchall()
    
    sql = "SELECT name FROM users WHERE auth = " + steamid3 + ";"
    conn = mysql.connector.connect(**db)
    cursor = conn.cursor()
    cursor.execute(sql)
    player = cursor.fetchone()
    
    text = []
    text2 = ""
    embed = []
    pages = 0
    for count, row in enumerate(results, 1):
        time = formatSeconds(row[0])   

        text2 += "\n" + row[1] + ": " + str(time) 
      
        if count % 25 == 0:
            pages += 1
            text.append(text2)
            text2 = ""
            
    text.append(text2) #final page of maps if count not multiple of 25
    pages += 1
    
    emojis = ['\u25c0', '\u25b6']
    
    embed=discord.Embed(title="MAP RECORDS\nPlayer: " + player[0], description=text[0], color=0xda190b)
    msg = await ctx.send(embed=embed) 
    
    def check(reaction, user):
        return user == ctx.message.author and (str(reaction.emoji) in emojis and reaction.message.id == msg.id)
    
    j = 0
    while True:
        await msg.add_reaction('\u25c0')
        await msg.add_reaction('\u25b6')
        
        reaction, user = await bot.wait_for('reaction_add', check=check)
        if str(reaction.emoji) == "\u25c0":  
            if j == 0:
                j = pages - 1
                embed=discord.Embed(title="MAP RECORDS\nPlayer: " + player[0], description=text[j], color=0xda190b)
                await msg.edit(embed=embed)
            else:
                j -= 1
                embed=discord.Embed(title="MAP RECORDS\nPlayer: " + player[0], description=text[j], color=0xda190b)
                await msg.edit(embed=embed)
            await msg.clear_reactions()
        if str(reaction.emoji) == "\u25b6":
            if j == pages - 1:
                j = 0
                embed=discord.Embed(title="MAP RECORDS\nPlayer: " + player[0], description=text[j], color=0xda190b)
                await msg.edit(embed=embed)
            else:
                j += 1
                embed=discord.Embed(title="MAP RECORDS\nPlayer: " + player[0], description=text[j], color=0xda190b)
                await msg.edit(embed=embed)
            await msg.clear_reactions()
    
    conn.close()
    cursor.close()
  
@records.error
async def records_cooldown(ctx, error):
    if isinstance(error, commands.CommandOnCooldown):
        msg = 'This command is on cooldown, please try again in {:.2f}s'.format(error.retry_after)
        await ctx.send(msg)
        
async def printRecord(ctx, mapname, track):
    if mapname.startswith('"'):
        sql = "SELECT time, jumps, sync, strafes, date, map, u.name, p.auth FROM " + TABLE_PREFIX + "playertimes p, " + TABLE_PREFIX + "users u WHERE map = " + str(mapname) + " AND track = " + str(track) + " AND style = 0 AND u.auth = p.auth ORDER BY time ASC LIMIT 1"
    else:
        sql = "SELECT time, jumps, sync, strafes, date, map, u.name, p.auth FROM " + TABLE_PREFIX + "playertimes p, " + TABLE_PREFIX + "users u WHERE map LIKE '%" + str(mapname) + "%' AND track = " + str(track) + " AND style = 0 AND u.auth = p.auth ORDER BY time ASC LIMIT 1"  
    
    conn = mysql.connector.connect(**db)
    cursor = conn.cursor()
    cursor.execute(sql)
    results = cursor.fetchone()
    conn.close()
    cursor.close()
    
    time = results[0]
    jumps = str(results[1])
    sync = str(results[2])
    strafes = str(results[3])
    timestamp = results[4]
    mapname = str(results[5])
    user = str(results[6]) 
    auth = results[7]
    
    if track == 0:
        trackName = "Map"
        trackColour = 0x1183f4
    else:
        trackName = "Bonus"
        trackColour = 0xe79f0c
        
    time = formatSeconds(results[0])
    date_time = datetime.fromtimestamp(timestamp)
    d = date_time.strftime("%d/%m/%Y")  
    link = "http://www.steamcommunity.com/profiles/" + str(SID.SteamID(auth))
    
    if STATS_PAGE:
        statslink = STATS_PAGE + "/?track=" + str(track) + "&map=" + mapname
        embed=discord.Embed(title=trackName + " Record", description="[" + mapname + "](" + statslink + ")", color=trackColour)
    if not STATS_PAGE:
        embed=discord.Embed(title=trackName + " Record", description=mapname, color=trackColour)
        
    embed.set_thumbnail(url=ICON)
    embed.set_footer(text="Join: steam://connect/" + IP + ":" + str(PORT))
    embed.add_field(name="Player‎‎", value="[" + user + "](" + link + ")", inline=True)
    embed.add_field(name="Time", value=time, inline=True)
    embed.add_field(name="Jumps", value=jumps, inline=True)
    embed.add_field(name="Sync", value=sync + "%", inline=True)
    embed.add_field(name="Strafes", value=strafes, inline=True)
    embed.add_field(name="Date", value=d, inline=True)
    
    await ctx.send(embed=embed)
  
@bot.command()
@commands.cooldown(1, 15, commands.BucketType.user)
async def ssj(ctx):
    num = random.randint(480,700)       
    
    await ctx.send(ctx.message.author.mention + " has a SSJ of " + str(num))
   
@ssj.error
async def ssj_cooldown(ctx, error):
    if isinstance(error, commands.CommandOnCooldown):
        msg = 'This command is on cooldown, please try again in {:.2f}s'.format(error.retry_after)
        await ctx.send(msg)
        
def formatSeconds(time):
    minutes = time / 60
    i, d = divmod(minutes, 1)
    seconds = d * 60
    minutes = math.trunc(i)
    seconds = round(seconds, 3)
    if time > 59:
        formatted = str(minutes) + ":" + str(seconds) 
    else:
        formatted = str(seconds) 
        
    return formatted
        
def formatSteamID3(arg):
    my_id = SteamID(arg) 
    steamid3 = my_id.steam3()   
    steamid3 = steamid3[5:]
    steamid3 = steamid3[:-1]
    
    return steamid3
    
bot.run(TOKEN)
