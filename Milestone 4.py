#!/usr/bin/python
import json
from bson import json_util
import bottle
from bottle import route, run, request, abort
import datetime
import pprint
from pymongo import MongoClient

connection = MongoClient('localhost', 27017)
db = connection['market']
collection = db['stocks']

#set up URI paths for REST service
@route('/hello', method='GET')
def get_hello():
  try:
    request.query.name
    name=request.query.name
    if name:
      string="{ hello: \""+request.query.name+"\"}"
    else:
      string="{ hello: }"
  except NameError:
    abort(404, 'No parameter for name %s' % name)
  return json.loads(json.dumps(string, indent=4, default=json_util.default))

@route('/strings', method='POST')
def post_stings():
  print request.json
  try:
    string1 = request.json.get('string1')
    string2 = request.json.get('string2')
    string="{ first: \""+str(string1)+"\", second: \""+str(string2)+"\" }"
  except NameError:
    abort(404, 'No parameter for string %s' % string)
  return json.loads(json.dumps(string, indent=4, default=json_util.default))

@route('/createStock', method='POST')
def create_document():
  try:
    collection.insert_one(request.json)
  except ValidationError as ve:
    abort(400, str(ve))
    return "false"
  return "true"

@route('/getStock', method='GET')
def read_document():
  try:
    request.query.Ticker
    ticker = request.query.Ticker
    result = collection.find_one({"Ticker": ticker})
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))

@route('/updateStock', method='GET')
def update_document():
  ticker = request.query.Ticker
  update = request.query.country
  try:
    collection.update_one({"Ticker": ticker}, {'$set' : {"Country": update}})
  except ValidationError as ve:
    abort(400, str(ve))
  return

@route('/deleteStock', method='GET')
def delete_document():
  ticker = request.query.Ticker
  try:
    collection.delete_one({"Ticker" : ticker})
  except ValidationError as ve:
    abort(400, str(ve))
  #pprint.pprint(collection.find_one(document))
  return
  
@route('/stockReport', method='GET')
def stock_report():
  try:
    list = request.json
    result = collection.find({"Ticker": {"$in" : list}})
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))

@route('/industryReport', method='GET')
def industry_report():
  try:
    industry = request.query.industry
    result = collection.find({"Industry" : industry}).sort({"Profit Margin" : -1}).limit(5)
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))

@route('/profitReport', method='GET')
def profit_report():
  try:
    profit = request.query.profit
    result = collection.find({"Profit Margin" : profit}).sort({"Ticker" : -1})
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))

@route('/sectorReport', method='GET')
def sector_report():
  try:
    sector = request.query.sector
    result = collection.find({"Sector" : sector}).sort({"Ticker" : -1}).limit(5)
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))

@route('/marginReport', method='GET')
def margin_report():
  try:
    upper = request.query.uppermargin
    lower = request.query.lowermargin
    result = collection.find({"Operating Margin" : { "$gte" : lower , "$lt" : upper}}).sort({"Ticker" : -1})
  except ValidationError as ve:
    abort(400, str(ve))
  return json.loads(json.dumps(result, indent=4, default=json_util.default))
  
if __name__=='__main__':
  #app.run(debug=True)
  run(host='localhost', port=8080)